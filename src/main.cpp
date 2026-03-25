/* TABLA DE PROGRESO (etapa -> etapa_str ; paso -> paso_str)
 { ciclo, etapa_str, paso_str }

 etapa_str = etapa + 2
 paso_str = (etapa * 10) + (11 + paso)
 paso: 0=LLENA, 1=DRENA, 2=CENTRI

 Etapa         etapa etapa_str LLENA(paso=0) DRENA(paso=1) CENTRI(paso=2)
 PRELAV        1     3          11            12            13
 LAV           2     4          14            15            16
 ENJ           3     5          17            18            19
 ENJX1         4     6          20            21            22
 ENJX2         5     7          23            24            25
 ENJF          6     8          26            27            28
 SPINF         7     9          00            00            29
 //v2
*/

#include <Arduino.h>
#define modelo 0            // modelo de tarjeta nueva 0 vieja 1
#define precio_on 0         // 0 sin precio , 3 solo precio iniciando
#define serial 1            // 1 activa la puerta 110V y puerta 24+,
#define continental 0       // 0 puerta continental off, 1 puerta continental on
#define activacion_puerta 0 // 0 puerta speedquen 110, 1 puerta continental
#define amortiguador 0      // activa acomodo de ropa 1 y 0 desactiva
#include <TM1637Display.h>
#include "Display.h"
#include "Button.h"
#include "Menu.h"
#include "Led.h"
#include "Seleccion_parametros.h"
#include "Lavado.h"
#include "EEPROM.h"
#include <Wire.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/wdt.h>

static bool ciclo_activo = false;
// ====== WDT policy: kick only if system progresses ======
static volatile uint8_t wd_heartbeat = 0; // lo incrementas cuando hay "vida"
static uint8_t wd_heartbeat_seen = 0;     // último heartbeat que ya pateamos

static inline void wd_mark_tick_1s(void)
{
  wd_heartbeat++; // NO usa millis
}

// En ciclo: solo patear si el tick vive.
// Fuera de ciclo: patear normal (menús/idle).
static inline void WD_KICK(void)
{
  if (!ciclo_activo)
  {
    wdt_reset();
    return;
  }

  const uint8_t hb = wd_heartbeat;
  if (hb != wd_heartbeat_seen)
  {
    wd_heartbeat_seen = hb;
    wdt_reset();
  }
}

uint8_t mcusr_mirror __attribute__((section(".noinit"))) __attribute__((used));

void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init3"))) __attribute__((used));
void get_mcusr(void)
{
  mcusr_mirror = MCUSR;
  MCUSR = 0;
  wdt_disable();
}

static uint8_t g_reset_cause = 0;

static void debug_capturar_reset(void)
{
  g_reset_cause = mcusr_mirror; // usamos el valor que se guardó en init3
}

#define PRESOSTATO A0
#define MOTOR_BAJA 34 // motor baja
#define SECUENCIA 39  // cambio de secuencia
#define MOTOR_ALTA 28 // motor alta
#define PUERTA 22
#define DESAGUE 26
#define TAMBOR_DEPOSITO1_FRIA 36
#define TAMBOR_DEPOSITO1_CALIENTE 44
#define DEPOSITO2 23
#define DEPOSITO3 24
#define BUTT_TOP 30   // button top-------------FALTA DEFINIRLOS
#define BUTT_BOT A2   // button bottom------------- FALTA DEFINIRLOS
#define BUTT_nivel 11 // button bottom------------- FALTA DEFINIRLOS
#if serial == 1
#define PUERTA_ON_110 50  // MISO
#define PUERTA_OFF_110 51 // MOSI
#define CONTROL_VAC 52    // SCK
#endif
#define moneda_ A3
#define CLK 4
#define DIO 2
TM1637Display ddisplay(CLK, DIO);
#define ciclo 7
#define temper 8
#define agua 9
#define inicio 10
#define tciclo_LED 25
#define ttemperatura_LED 27
#define tagua_LED 29
#define tinicio_LED 32
#define prelavadoLED 37
#define lavadoLED 35
#define enjuagueLED 33
#define enjuagueEXLED 38
#define enjugauefinLED 41
#define cenrifugadoLED 40
#define maquina_on 6

static void debug_mostrar_reset(void)
{
  debug_capturar_reset();

  int code = 0;
  const char *txt = "UNK";

  if (g_reset_cause & _BV(PORF))
  {
    code = 1;
    txt = "POR";
  }
  else if (g_reset_cause & _BV(BORF))
  {
    code = 2;
    txt = "BOR";
  }
  else if (g_reset_cause & _BV(EXTRF))
  {
    code = 3;
    txt = "EXT";
  }
  else if (g_reset_cause & _BV(WDRF))
  {
    code = 4;
    txt = "WDT";
  }

  if (code == 0)
  {
    // Nada interesante, no estorbamos a la máquina
    return;
  }

  // Usa el MISMO objeto de display que uses para PAY; si es "display", usa este:
  ddisplay.clear();
  ddisplay.setBrightness(0x0f);
  ddisplay.showNumberDec(code, true, 1, 3);

  unsigned long t0 = millis();
  while (millis() - t0 < 2000UL)
  {
    WD_KICK();
  }

  ddisplay.clear();
}

#define eprom 0 // actualizar parametros 1 ,, para bloquearlos 0///////////////////////
#define opl 1   // para quitar comunicacion y jale opl, 0 ,1 para activar comunicacion

// --- reset remoto del medidor de agua ---
static void medidor_reset_remote(void)
{
  // Vacía cualquier basura pendiente en RX
  while (Serial.available() > 0)
    (void)Serial.read();

  // Envía el comando de reset
  Serial.print("<RST>");

  // Da un pequeño margen para que el medidor procese y responda {OK}\r\n
  unsigned long t0 = millis();
  while (millis() - t0 < 200UL)
  {
    WD_KICK();
    // Limpia el ACK/eco si llega algo
    if (Serial.available() > 0)
      (void)Serial.read();
  }
  // A partir de aquí el medidor seguirá transmitiendo [0][1]... desde cero
}

static void mostrar_total_ciclo_display(unsigned long L)
{
  // Cabe en 4 dígitos (0..9999). Si te pasas, muestra 9999.
  if (L > 9999UL)
    L = 9999UL;

  ddisplay.clear();
  display.setBrightness(0x0f);
  display.showNumberDec((int)L, true, 4, 0);

  // Espera ~3 s alimentando el WDT
  unsigned long t0 = millis();
  while (millis() - t0 < 3000UL)
  {
    WD_KICK();
  }

  ddisplay.clear();
}

static inline void medidor_dump_tick()
{
  while (Serial.available() > 0)
  {
    char c = (char)Serial.read(); // viene del medidor por UART0
  }
}

static char md_buf[8];
static uint8_t md_len = 0;
static bool md_in = false;

static inline bool medidor_read_brackets(int *outL)
{
  while (Serial.available() > 0)
  {
    char c = (char)Serial.read();

    if (!md_in)
    {
      if (c == '[')
      {
        md_in = true;
        md_len = 0;
      }
      continue;
    }

    if (c == ']')
    {
      md_buf[(md_len < sizeof(md_buf) - 1) ? md_len : (sizeof(md_buf) - 1)] = '\0';
      long v = 0;
      bool ok = (md_len > 0);
      for (uint8_t i = 0; i < md_len && ok; ++i)
      {
        char d = md_buf[i];
        if (d < '0' || d > '9')
          ok = false;
        else
          v = v * 10 + (d - '0');
      }
      md_in = false;
      md_len = 0;
      if (ok)
      {
        *outL = (int)v;
        return true;
      }
      return false;
    }

    if (c >= '0' && c <= '9')
    {
      if (md_len < sizeof(md_buf) - 1)
        md_buf[md_len++] = c;
      else
      {
        md_in = false;
        md_len = 0;
      } // overflow → reset
    }
    else if (c == ' ' || c == '\r' || c == '\n' || c == '\t')
    {
      // ignorar blancos
    }
    else
    {
      md_in = false;
      md_len = 0; // basura -> descartar mensaje
    }
  }
  return false;
}

// --- último valor del medidor ---
static int md_last_L = 0;
static bool md_have_L = false;

// Debe existir: bool medidor_read_brackets(int *outL);
static inline void medidor_tick_update_last()
{
  int L;
  if (medidor_read_brackets(&L))
  {
    md_last_L = L;
    md_have_L = true;
  }
}

// --- litros objetivo por ETAPA (EEPROM 200..205) ---
static inline int objetivo_litros_por_etapa(uint8_t etapa_1a6)
{
  switch (etapa_1a6)
  {
  case 1:
    return EEPROM.read(200); // PRELAV
  case 2:
    return EEPROM.read(201); // LAV
  case 3:
    return EEPROM.read(202); // ENJ
  case 4:
    return EEPROM.read(203); // ENJX1
  case 5:
    return EEPROM.read(204); // ENJX2
  case 6:
    return EEPROM.read(205); // ENJF
  default:
    return 0;
  }
}

// --- modo ahorro/carry (EEPROM 206: 0=OFF, 1=ON) ---
static inline bool modo_carry_on() { return EEPROM.read(206) != 0; }

// --- estado por PASO_LLENA ---
static bool fill_step_started = false;
static long step_baseline_L = 0; // L al entrar al paso
static long step_obj_L = 0;      // L a meter en este paso

// --- carry ENJX2 -> ENJF ---
static bool ex2_residual_valid = false;
static long ex2_residual_L = 0; // lo metido en ENJX2

// --- ACUMULADO DE AGUA DEL CICLO (en litros enteros tal como llegan) ---

static bool ciclo_l0_capturado = false;
static long ciclo_prev_total_L = 0;     // último [L] visto
static unsigned long ciclo_total_L = 0; // litros acumulados del ciclo

static inline void medidor_ciclo_update_with_L(int L)
{
  if (!ciclo_activo)
    return;

  if (!ciclo_l0_capturado)
  {
    ciclo_prev_total_L = L; // primera muestra del ciclo
    ciclo_l0_capturado = true;
  }
  else
  {
    long inc = (L >= ciclo_prev_total_L) ? (L - ciclo_prev_total_L) : L; // tolera reinicio
    if (inc > 0)
      ciclo_total_L += (unsigned long)inc;
    ciclo_prev_total_L = L;
  }
}

// Envolver la lectura para actualizar el acumulador automáticamente
static inline bool medidor_read_brackets_and_ciclo(int *outL)
{
  int L;
  if (!medidor_read_brackets(&L))
    return false;
  medidor_ciclo_update_with_L(L);
  *outL = L;
  return true;
}

// APIs cortas para arrancar/cerrar ciclo y consultar total
static inline void ciclo_begin_agua(void)
{
  ciclo_activo = true;
  wd_mark_tick_1s(); // <-- agrega esto

  ciclo_l0_capturado = false;
  ciclo_prev_total_L = 0;
  ciclo_total_L = 0;
}
static inline void ciclo_end_agua(void) { ciclo_activo = false; }
static inline unsigned long ciclo_total_agua_L(void) { return ciclo_total_L; }

int DEFAULT_tipo_ciclo = 1;
int DEFAULT_tipo_temperatura = 1;
int DEFAULT_nivel_agua = 1;
Button tipo_ciclo(ciclo);
Button tipo_temperatura(temper);
Button nivel_agua(agua);
Button Inicio(inicio);
Menu menu_temp;
Menu menu_ciclo;
Menu menu_agua;
Led led(tciclo_LED, ttemperatura_LED, tagua_LED, tinicio_LED);
Lavado lavado(PRESOSTATO, MOTOR_BAJA, SECUENCIA, MOTOR_ALTA, PUERTA, DESAGUE, TAMBOR_DEPOSITO1_FRIA, TAMBOR_DEPOSITO1_CALIENTE, DEPOSITO2, DEPOSITO3, BUTT_TOP, BUTT_BOT, PUERTA_ON_110, PUERTA_OFF_110, CONTROL_VAC);
int cambio = 1;
int cambio_2 = 1;
int ciclo_str = 0;
int etapa_str = 0;
int paso_str = 0;
int status_etapa = 1;
int status_etapa_1 = 1;
int etapa_2 = 0;
int contador_errores_e6 = 0;
int contador_errores_e5 = 0;
int continua = 1;
int activacion_2 = 0;
int auxiliar_puerta_error = 0;
static bool instruccionEnviada29 = false;
const unsigned long tiempoDeseado = 20000;
unsigned long tiempoInicio;
unsigned long lastTime = 0;
int index = 0;
int contador_moneda = 0;
bool dato_moneda = HIGH;
bool datoAnterior_moneda = HIGH;
int aux_55 = 0;
int ciclo_1 = EEPROM.read(162);
int ciclo_2 = EEPROM.read(163);
int ciclo_3 = EEPROM.read(164);
int ciclo_4 = EEPROM.read(190);
int precio_tibia = EEPROM.read(165);
int precio_caliente = EEPROM.read(166);
int nivel_2 = EEPROM.read(167);
int nivel_3 = EEPROM.read(168);
int valor_moneda = EEPROM.read(169);
int posicion1 = 0;
String palabra1 = "";
int aa, aa1;
int aux1 = 0, aux2 = 101;
int precio_temp = 0;
int precio_ciclo = 0;
int precio_nivel = 0;
int k = 0;                            //--------------------------> debe estar en cero pero va a hacer en dos para empezarr comunicacon
unsigned long lastDebounceTime = 0;   // the last time the output pin was toggled
unsigned long debounceDelay = 200000; // the debounce time; increase if the output flickers
String recived;
int activacion = 0;
int total = 0;   // the running total
int average = 0; // the average
String cc = "";
float voltage;
int contador_programcion = 0;
int val = 0;
int prog = 0;
int etapa = 1;
long int segundostotalB = 0;
unsigned long segundosAux = 0;
unsigned long tiempo_aux2 = 0;
int paso = 0;
int contadorP = 0;
int dato_llenado;
int dato_error;
int datoAnterior_error = 0;
int contador_error_llenado;
int tiempo_error_llenado = EEPROM.read(161);
bool t = 1;
int llenado_error = 1;
int error_llenado = 0;
int dato_desague;
int datoAnterior_llenado = 0;
int contador_llenado = 1;
int datoAnterior_desague = 0;
int contador_desague = 0;
bool llenado = 1;
bool desague = 1;
int datoAnteriorP = 0;
int datoP;
int temperatura = 0;
int ahoras = 0;
int aminutos = 0;
int asegundos = 0;
int segundostotal = 0;
unsigned long time = 0;
int dato2;
int datoAnterior2;
int LLENADO_AGIpre;
int DESAGUE_AGIpre;
int CENTRIFUpre;
int nivelde_llenado_prelavado;
int TIEMPO_PRELAVADO;
int LLENADO_AGIlav;
int DESAGUE_AGIlav;
int CENTRIFUlav;
int nivelde_llenado_lavado;
int TIEMPO_LAVADO;
int LLENADO_AGIeng;
int DESAGUE_AGIeng;
int CENTRIFUeng;
int nivelde_llenado_enjugaue;
int TIEMPO_ENJUAGUE;
int LLENADO_AGIex1;
int DESAGUE_AGIex1;
int CENTRIFUex1;
int nivel_de_llenado_ennjuague_extra_1;
int TIEMPO_ENJUAGUE_EXTRA_1;
int LLENADO_AGIex2;
int DESAGUE_AGIex2;
int CENTRIFUex2;
int nivel_de_llenado_ennjuague_extra_2;
int TIEMPO_ENJUAGUE_EXTRA_2;
int LLENADO_AGIfin;
int DESAGUE_AGIfin;
int CENTRIFUfin;
int nivel_de_llenado_ennjuague_final;
int TIEMPO_ENJUAGUE_FINAL;
int TIEMPO_CENTRIFUGADO_FINAL;
int nivel_de_llenado_centrifugado;
int TIEMPO;
int tiempo_giro_izquierda;
int tiempo_giro_derecha;
int tiempo_reposo;
void parametros()
{
  LLENADO_AGIpre = LLENADO_AGIpre_1;
  DESAGUE_AGIpre = LLENADO_AGIpre_1;
  CENTRIFUpre = CENTRIFUpre_1;
  nivelde_llenado_prelavado = nivelde_llenado_prelavado_1;
  TIEMPO_PRELAVADO = LLENADO_AGIpre + DESAGUE_AGIpre + CENTRIFUpre;
  LLENADO_AGIlav = LLENADO_AGIlav_1;
  DESAGUE_AGIlav = DESAGUE_AGIlav_1;
  CENTRIFUlav = CENTRIFUlav_1;
  nivelde_llenado_lavado = nivelde_llenado_lavado_1;
  TIEMPO_LAVADO = LLENADO_AGIlav + DESAGUE_AGIlav + CENTRIFUlav;
  LLENADO_AGIeng = LLENADO_AGIeng_1;
  DESAGUE_AGIeng = DESAGUE_AGIeng_1;
  CENTRIFUeng = CENTRIFUeng_1;
  nivelde_llenado_enjugaue = nivelde_llenado_enjugaue_1;
  TIEMPO_ENJUAGUE = LLENADO_AGIeng + DESAGUE_AGIeng + CENTRIFUeng;
  LLENADO_AGIex1 = LLENADO_AGIex1_1;
  DESAGUE_AGIex1 = DESAGUE_AGIex1_1;
  CENTRIFUex1 = CENTRIFUex1_1;
  nivel_de_llenado_ennjuague_extra_1 = nivel_de_llenado_ennjuague_extra_1_1;
  TIEMPO_ENJUAGUE_EXTRA_1 = LLENADO_AGIex1 + DESAGUE_AGIex1 + CENTRIFUex1;
  LLENADO_AGIex2 = LLENADO_AGIex2_1;
  DESAGUE_AGIex2 = DESAGUE_AGIex2_1;
  CENTRIFUex2 = CENTRIFUex2_1;
  nivel_de_llenado_ennjuague_extra_2 = nivel_de_llenado_ennjuague_extra_2_1;
  TIEMPO_ENJUAGUE_EXTRA_2 = LLENADO_AGIex2 + DESAGUE_AGIex2 + CENTRIFUex2;
  LLENADO_AGIfin = LLENADO_AGIfin_1;
  DESAGUE_AGIfin = DESAGUE_AGIfin_1;
  CENTRIFUfin = CENTRIFUfin_1;
  nivel_de_llenado_ennjuague_final = nivel_de_llenado_ennjuague_final_1;
  TIEMPO_ENJUAGUE_FINAL = LLENADO_AGIfin + DESAGUE_AGIfin + CENTRIFUfin;
  TIEMPO_CENTRIFUGADO_FINAL = TIEMPO_CENTRIFUGADO_FINAL_1;
  nivel_de_llenado_centrifugado = nivel_de_llenado_centrifugado_1;
  TIEMPO = TIEMPO_PRELAVADO + TIEMPO_LAVADO + TIEMPO_ENJUAGUE + TIEMPO_ENJUAGUE_EXTRA_1 + TIEMPO_ENJUAGUE_EXTRA_2 + TIEMPO_ENJUAGUE_FINAL + TIEMPO_CENTRIFUGADO_FINAL;
  tiempo_giro_izquierda = tiempo_giro_izquierda_1;
  tiempo_giro_derecha = tiempo_giro_derecha_1;
  tiempo_reposo = tiempo_reposo_1;
  temperatura = temperatura_1;
}

static inline void seleccionar_valvulas_por_etapa_temp(int etapa, int temperatura)
{
  if (etapa == 6)
  { // Enjuague final en cualquier temperatura
    lavado.enjuague_final();
    return;
  }
  switch (temperatura)
  {
  case 0:
    lavado.agua_fria();
    break;
  case 1:
    lavado.agua_tibia();
    break;
  case 2:
    lavado.agua_caliente();
    break;
  default:
    lavado.val_off();
    break;
  }
}

enum : int
{
  PRELAV = 1,
  LAV = 2,
  ENJ = 3,
  ENJX1 = 4,
  ENJX2 = 5,
  ENJF = 6,
  SPINF = 7
};
enum : int
{
  PASO_LLENA = 0,
  PASO_DRENA = 1,
  PASO_CENTRI = 2
};


static inline void valvulas_off(void)
{
  lavado.val_off();
}
///////////////////////////////////////////////////////////////////////////////////////////
static const uint8_t T_DEPOSITO_S = 20U;   // ventana de detergente/suavizante
static const uint8_t T_L6_CALIENTE_S = 10U; // temp caliente  ajustar

static inline void fill_apply_destination(int etapa, uint32_t t_fill_s)
{
  // Estado base
  lavado.L7_OFF();
  lavado.L8_OFF();

  switch (etapa)
  {
  case PRELAV:
  case LAV:
    // primeros 10 s: jabonera detergente
    // después: tina directa
    if (t_fill_s >= T_DEPOSITO_S)
    {
      lavado.L7_ON();
    }
    break;

  case ENJ:
  case ENJX1:
  case ENJX2:
    // siempre directo a tina
    lavado.L7_ON();
    break;

  case ENJF:
    // primeros 10 s: suavizante
    if (t_fill_s < T_DEPOSITO_S)
    {
      lavado.L8_ON();
    }
    else
    {
      lavado.L7_ON();
    }
    break;

  default:
    lavado.L7_OFF();
    lavado.L8_OFF();
    break;
  }
}

static inline void fill_apply_temperature(int etapa, int temperatura, uint32_t t_fill_s)
{
  // Estado base: evitar arrastres viejos
  lavado.L5_OFF();
  lavado.L6_OFF();

  // =========================================================
  // ENJUAGUES INTERMEDIOS: directos a tina y fríos
  // =========================================================
  if ((etapa == ENJ) || (etapa == ENJX1) || (etapa == ENJX2))
  {
    // En tu definición actual, en tina directa fría/tibia no usan L5 ni L6
    return;
  }

  // =========================================================
  // ENJUAGUE FINAL
  // =========================================================
  if (etapa == ENJF)
  {
    if (t_fill_s < T_DEPOSITO_S)
    {
      // Primeros 10 s: suavizante con fría
      lavado.L6_ON();
    }
    else
    {
      // Después: tina directa fría, sin L5 ni L6
      // ambos ya quedaron OFF arriba
    }
    return;
  }

  // =========================================================
  // PRELAV / LAV
  // =========================================================
  if ((etapa == PRELAV) || (etapa == LAV))
  {
    if (t_fill_s < T_DEPOSITO_S)
    {
      // ----- FASE JABONERA -----
      switch (temperatura)
      {
      case 0: // FRIA
        lavado.L6_ON();
        break;

      case 1: // TIBIA
        lavado.L6_ON();
        lavado.L5_ON();
        break;

      case 2: // CALIENTE
        lavado.L5_ON();

        if (t_fill_s < T_L6_CALIENTE_S)
        {
          lavado.L6_ON();
        }
        break;

      default:
        // ambos OFF
        break;
      }
    }
    else
    {
      // ----- FASE TINA DIRECTA -----
      // Fría y tibia: L5 OFF, L6 OFF
      // Caliente: solo L5 ON
      if (temperatura == 2)
      {
        lavado.L5_ON();
      }
    }

    return;
  }

  // =========================================================
  // Cualquier otro caso: todo OFF
  // =========================================================
}

static inline void fill_apply_strategy(int etapa, int temperatura, uint32_t t_fill_s)
{
  fill_apply_destination(etapa, t_fill_s);
  fill_apply_temperature(etapa, temperatura, t_fill_s);
}

////////////////////////////////////////////////////////////////////////////////////////

static inline void manejar_valvulas(int nivel_objetivo, int nivel_actual, int etapa, int temperatura)
{
  if (nivel_objetivo > nivel_actual)
  {
    seleccionar_valvulas_por_etapa_temp(etapa, temperatura);
  }
  else
  {
    valvulas_off();
  }
}
static inline bool nivel_alcanzado(void)
{
  return (digitalRead(A0) == 0);
}
static int __mm_prev = -1;
static inline void ui_mostrar_minutos(int mm)
{

  if (mm < 0)
    mm = 0;
  if (mm > 99)
    mm = 99;
  if (mm == __mm_prev)
    return;
  __mm_prev = mm;
  display.setBrightness(0x0f);
  uint8_t segs[4];
  segs[0] = 0x00;                         // dígito 0 apagado
  segs[1] = 0x00;                         // dígito 1 apagado
  segs[2] = display.encodeDigit(mm / 10); // decenas
  segs[3] = display.encodeDigit(mm % 10); // unidades
  display.setSegments(segs);
}


/* =======================
 MAPAS Y FORMATOS
 =======================

 PROGRESO (curly braces):
 { ciclo, etapa_str, paso_str }
 etapa_str = etapa + 2
 paso_str = (etapa * 10) + (11 + paso)
 paso: 0=LLENA, 1=DRENA, 2=CENTRI

 EVENTOS GLOBALES (brackets):
 [ evento, ciclo, 0 ]
 evento: 0=INICIO (handshake),
 1=E1 (error llenado),
 5=E5 (puerta – secuencia),
 6=E6 (puerta – bloqueo)
*/

static inline int codigo_etapa_str(int etapa)
{
  return etapa + 2;
}

static inline int codigo_paso_str(int etapa, int paso)
{
  if (etapa == SPINF)
  {
    // SPINF: solo CENTRI existe
    return (paso == PASO_CENTRI) ? 29 : 0; // LLENA/DRENA -> 0
  }

  // PRELAV..ENJF (1..6) y paso 0..2
  if (etapa >= PRELAV && etapa <= ENJF &&
      paso >= PASO_LLENA && paso <= PASO_CENTRI)
  {
    return 10 + ((etapa - PRELAV) * 3) + (paso + 1);
  }

  return 0; // fuera de rango → 0
}

// ---- PROGRESO: envía {ciclo, etapa_str, paso_str} una sola vez por cambio
static long __clave_progreso_prev = -1;
static inline void enviar_progreso_una_vez(int etapa, int paso)
{
  const int ciclo_id = DEFAULT_tipo_ciclo;
  const int es = codigo_etapa_str(etapa);
  const int ps = codigo_paso_str(etapa, paso);
  const long clave = ((long)ciclo_id << 16) | (es << 8) | ps;
  if (__clave_progreso_prev != clave)
  {
    Serial2.print("{" + String(ciclo_id) + "," + String(es) + "," + String(ps) + "}\n");
    __clave_progreso_prev = clave;
  }
}

// ---- EVENTOS: envía [evento, ciclo, 0]
enum : uint8_t
{
  EV_INICIO = 0, // Handshake de arranque
  EV_E1 = 1,     // Error llenado
  EV_E5 = 5,     // Error puerta (secuencia)
  EV_E6 = 6,     // Error puerta (bloqueo)

  EVT_FIN = 7
};
static bool __ltt_enviado = false;
static inline void especiales_reset() { __ltt_enviado = false; }
static inline void enviar_evento(uint8_t ev_code)
{

  if (ev_code == EVT_FIN)
  {
    unsigned long L = ciclo_total_agua_L(); // litros acumulados del ciclo
    Serial2.print("[" + String(ev_code) + "," + String(DEFAULT_tipo_ciclo) + ",LTT" + String(L) + "]\n");
    __ltt_enviado = true; // por si usas el sender LTT independiente, evita duplicar
  }
  else
  {
    Serial2.print("[" + String(ev_code) + "," + String(DEFAULT_tipo_ciclo) + ",0]\n");
  }
}

static void lanzar_error_llenado_bloqueante(void)
{
  lavado.drenado();
  lavado.STOP_M();
  valvulas_off();
  ddisplay.clear();
  display.setBrightness(0x0f);
  display.setSegments(SEG_E1);
  enviar_evento(EV_E1);

  delay(3000);
  if (aux_55 == 0)
  {
    lavado.PUERTA_OFF();
#if continental == 1
    lavado.r_continental_off_1();
    delay(200);
    lavado.r_continental_on_2();
    delay(10000);
    lavado.r_continental_off_2();
#endif
  }
  aux_55 = 1;
  while (1)
  {
    WD_KICK();
    tone(buzzer, 2000);
    delay(1000);
    noTone(buzzer);
    delay(1000);
  }
}

static void motor_step_int(int *pCont, int t_izq, int t_der, int t_rep)
{
  if (*pCont < 1)
  {
    *pCont = 1;
  }
  if (llenado == 1)
  {
    if (t == 0)
    {
      if (*pCont <= t_izq)
      {
        lavado.IZQUIERDA_M();
      }
      else
      {
        *pCont = 1;
        t = 1; // pasar a reposo
      }
    }
    else
    { // t == 1
      if (*pCont <= t_rep)
      {
        lavado.STOP_M();
      }
      else
      {
        t = 0; // volver a giro
        *pCont = 1;
        llenado = 0; // CAMBIO DE SENTIDO: siguiente bloque será DERECHA
      }
    }
  }
  else
  { // llenado == 0 → bloque DERECHA
    if (t == 0)
    {
      if (*pCont <= t_der)
      {
        lavado.DERECHA_M();
      }
      else
      {
        *pCont = 1;
        t = 1; // pasar a reposo
      }
    }
    else
    { // t == 1
      if (*pCont <= t_rep)
      {
        lavado.STOP_M();
      }
      else
      {
        t = 0; // volver a giro
        *pCont = 1;
        llenado = 1; // CAMBIO DE SENTIDO: siguiente bloque será IZQUIERDA
      }
    }
  }
}

/*
void llenado_mojado(int dato_llenado,
                    int nivelde_llenado_prelavado,
                    int tiempo_giro_izquierda,
                    int tiempo_giro_derecha,
                    int tiempo_reposo,
                    int tiempo_aux2,
                    int LLENADO_AGIpre,
                    int temperatura,
                    int ETAPA)
{
  lavado.no_drenado();
  manejar_valvulas(nivelde_llenado_prelavado, average, ETAPA, temperatura);

  if (dato_llenado != datoAnterior_llenado)
  {
    // solo para mostrar lo que llega en cualquier tick

    if (average <= nivelde_llenado_prelavado && llenado_error == 1)
    {
      time = 0;
      while (1) // BLOQUEANTE (igual que tenías)
      {

        // --- leer litros y MOSTRAR ---
        int L;
        if (medidor_read_brackets_and_ciclo(&L))
        {
          display.setBrightness(0x0f);
          display.showNumberDec(L, true, 4, 0);

          // --------- NUEVO: CORTE POR LITROS ----------
          if (!fill_step_started)
          {
            // primer valor del paso: baseline y objetivo por etapa
            step_baseline_L = L;
            step_obj_L = objetivo_litros_por_etapa((uint8_t)ETAPA); // EEPROM 200..205
            fill_step_started = true;
          }
          else if (step_obj_L > 0)
          {
            long delta = (long)L - step_baseline_L; // litros metidos en ESTE paso
            if (delta >= step_obj_L)
            {
              // mismo cierre que se hace cuando "nivel_alcanzado()"
              valvulas_off();
              average = nivelde_llenado_prelavado + 1;
              contador_llenado = 1;
              t = 1;
              llenado_error = 0;
              llenado = 1;
              break; // salir del while(1)
            }
          }
          // --------- FIN NUEVO ----------
        }

        WD_KICK();
        time = millis() / 1000;
        dato_error = time;

        if (dato_error != datoAnterior_error)
        {
          wd_mark_tick_1s();
          contador_error_llenado++;
          contador_llenado++;
          motor_step_int(&contador_llenado, tiempo_giro_izquierda, tiempo_giro_derecha, tiempo_reposo);

          if (contador_error_llenado >= (uint32_t)tiempo_error_llenado * 60U)
          {
            lanzar_error_llenado_bloqueante();
          }

          if (nivel_alcanzado())
          {
            valvulas_off();
            average = nivelde_llenado_prelavado + 1;
            contador_llenado = 1;
            t = 1;
            llenado_error = 0;
            llenado = 1;
            break;
          }
        }
        datoAnterior_error = dato_error;
      }
    }

    // tu lógica de motor tal cual
    if (llenado == 1)
    {
      contador_llenado++;
      motor_step_int(&contador_llenado, tiempo_giro_izquierda, tiempo_giro_derecha, tiempo_reposo);
    }
    else
    {
      contador_llenado++;
      motor_step_int(&contador_llenado, tiempo_giro_izquierda, tiempo_giro_derecha, tiempo_reposo);
    }
  }

  datoAnterior_llenado = dato_llenado;
}
*/

void llenado_mojado(int dato_llenado,
                    int nivelde_llenado_prelavado,
                    int tiempo_giro_izquierda,
                    int tiempo_giro_derecha,
                    int tiempo_reposo,
                    int tiempo_aux2,
                    int LLENADO_AGIpre,
                    int temperatura,
                    int ETAPA)
{
  (void)tiempo_aux2;
  (void)LLENADO_AGIpre;

  lavado.no_drenado();

  if (dato_llenado != datoAnterior_llenado)
  {
    if (average <= nivelde_llenado_prelavado && llenado_error == 1)
    {
      time = 0;

      while (1)
      {
        bool corte_litros = false;
        bool corte_presostato = false;

        // IMPORTANTE:
        // Como aquí estás en un while bloqueante, el tiempo real de llenado
        // lo tomamos de contador_error_llenado, no de tiempo_aux2.
        uint32_t t_fill_s = (uint32_t)contador_error_llenado;

        // =========================================================
        // 1) Aplicar estrategia nueva de llenado
        // =========================================================
        fill_apply_strategy(ETAPA, temperatura, t_fill_s);

        // =========================================================
        // 2) Leer litros y evaluar corte por litros
        // =========================================================
        int L;
        if (medidor_read_brackets_and_ciclo(&L))
        {
          display.setBrightness(0x0f);
          display.showNumberDec(L, true, 4, 0);

          if (!fill_step_started)
          {
            step_baseline_L = L;
            step_obj_L = objetivo_litros_por_etapa((uint8_t)ETAPA);
            fill_step_started = true;
          }
          else if (step_obj_L > 0)
          {
            long delta = (long)L - step_baseline_L;
            if (delta >= step_obj_L)
            {
              corte_litros = true;
            }
          }
        }

        // =========================================================
        // 3) Evaluar corte por presostato
        // =========================================================
        if (nivel_alcanzado())
        {
          corte_presostato = true;
        }

        // =========================================================
        // 4) El corte manda por encima de todo
        // =========================================================
        if (corte_litros || corte_presostato)
        {
          lavado.FILL_OFF();

          average = nivelde_llenado_prelavado + 1;
          contador_llenado = 1;
          t = 1;
          llenado_error = 0;
          llenado = 1;
          break;
        }

        // =========================================================
        // 5) Mantener watchdog / tiempo / motor
        // =========================================================
        WD_KICK();

        time = millis() / 1000;
        dato_error = time;

        if (dato_error != datoAnterior_error)
        {
          wd_mark_tick_1s();
          contador_error_llenado++;
          contador_llenado++;

          motor_step_int(&contador_llenado,
                         tiempo_giro_izquierda,
                         tiempo_giro_derecha,
                         tiempo_reposo);

          if (contador_error_llenado >= ((uint32_t)tiempo_error_llenado * 60U))
          {
            lanzar_error_llenado_bloqueante();
          }
        }

        datoAnterior_error = dato_error;
      }
    }

    // Dejo tu lógica de motor tal cual para no mover más de lo necesario
    if (llenado == 1)
    {
      contador_llenado++;
      motor_step_int(&contador_llenado,
                     tiempo_giro_izquierda,
                     tiempo_giro_derecha,
                     tiempo_reposo);
    }
    else
    {
      contador_llenado++;
      motor_step_int(&contador_llenado,
                     tiempo_giro_izquierda,
                     tiempo_giro_derecha,
                     tiempo_reposo);
    }
  }

  datoAnterior_llenado = dato_llenado;
}

void drenado_lavado(int dato_llenado,
                    int /*nivelde_llenado_prelavado*/,
                    int tiempo_giro_izquierda,
                    int tiempo_giro_derecha,
                    int tiempo_reposo,
                    int /*tiempo_aux2*/,
                    int /*LLENADO_AGIpre*/)
{
  lavado.drenado();
  if (dato_llenado != datoAnterior_llenado)
  {
    contador_llenado++;
    motor_step_int(&contador_llenado, tiempo_giro_izquierda, tiempo_giro_derecha, tiempo_reposo);
  }
  datoAnterior_llenado = dato_llenado;
}

inline int dur_llenado(int e)
{
  switch (e)
  {
  case PRELAV:
    return LLENADO_AGIpre;
  case LAV:
    return LLENADO_AGIlav;
  case ENJ:
    return LLENADO_AGIeng;
  case ENJX1:
    return LLENADO_AGIex1;
  case ENJX2:
    return LLENADO_AGIex2;
  case ENJF:
    return LLENADO_AGIfin;
  default:
    return 0;
  }
}
inline int dur_desague(int e)
{
  switch (e)
  {
  case PRELAV:
    return DESAGUE_AGIpre;
  case LAV:
    return DESAGUE_AGIlav;
  case ENJ:
    return DESAGUE_AGIeng;
  case ENJX1:
    return DESAGUE_AGIex1;
  case ENJX2:
    return DESAGUE_AGIex2;
  case ENJF:
    return DESAGUE_AGIfin;
  default:
    return 0;
  }
}
inline int dur_centri(int e)
{
  switch (e)
  {
  case PRELAV:
    return CENTRIFUpre;
  case LAV:
    return CENTRIFUlav;
  case ENJ:
    return CENTRIFUeng;
  case ENJX1:
    return CENTRIFUex1;
  case ENJX2:
    return CENTRIFUex2;
  case ENJF:
    return CENTRIFUfin;
  default:
    return (e == SPINF ? TIEMPO_CENTRIFUGADO_FINAL : 0);
  }
}
inline int nivel_objetivo(int e)
{
  switch (e)
  {
  case PRELAV:
    return nivelde_llenado_prelavado;
  case LAV:
    return nivelde_llenado_lavado;
  case ENJ:
    return nivelde_llenado_enjugaue;
  case ENJX1:
    return nivel_de_llenado_ennjuague_extra_1;
  case ENJX2:
    return nivel_de_llenado_ennjuague_extra_2;
  case ENJF:
    return nivel_de_llenado_ennjuague_final;
  default:
    return 0;
  }
}
inline void reset_estado_paso()
{
  t = 1;
  llenado_error = 1;
  error_llenado = 0;
  dato_error = 0;
  datoAnterior_error = 0;
  contador_error_llenado = 0;
  dato_llenado = 0;
  dato_desague = 0;
  datoAnterior_llenado = 0;
  contador_llenado = 0;
  datoAnterior_desague = 0;
  contador_desague = 0;
  llenado = 1;
  desague = 1;
  average = 0;
  tiempo_aux2 = 0;
  // --- litros por paso ---
  fill_step_started = false;
  step_baseline_L = 0;
  step_obj_L = 0;
}
inline void intentar_avanzar_etapa()
{
  if (etapa == PRELAV && segundosAux >= TIEMPO_PRELAVADO * 60)
  {
    etapa = LAV;
    segundosAux = 0;
    tiempo_aux2 = 0;
    paso = 0;
    average = 0;
  }
  else if (etapa == LAV && segundosAux >= TIEMPO_LAVADO * 60)
  {
    etapa = ENJ;
    segundosAux = 0;
    tiempo_aux2 = 0;
    paso = 0;
    average = 0;
  }
  else if (etapa == ENJ && segundosAux >= TIEMPO_ENJUAGUE * 60)
  {
    etapa = ENJX1;
    segundosAux = 0;
    tiempo_aux2 = 0;
    paso = 0;
    average = 0;
  }
  else if (etapa == ENJX1 && segundosAux >= TIEMPO_ENJUAGUE_EXTRA_1 * 60)
  {
    etapa = ENJX2;
    segundosAux = 0;
    tiempo_aux2 = 0;
    paso = 0;
    average = 0;
  }
  else if (etapa == ENJX2 && segundosAux >= TIEMPO_ENJUAGUE_EXTRA_2 * 60)
  {
    etapa = ENJF;
    segundosAux = 0;
    tiempo_aux2 = 0;
    paso = 0;
    average = 0;
  }
  else if (etapa == ENJF && segundosAux >= TIEMPO_ENJUAGUE_FINAL * 60)
  {
    etapa = SPINF;
    segundosAux = 0;
    tiempo_aux2 = 0;
    paso = 0;
    average = 0;
    // limpiar carry para el próximo ciclo
    ex2_residual_valid = false;
    ex2_residual_L = 0;
  }
  else if (etapa == SPINF && segundosAux >= TIEMPO_CENTRIFUGADO_FINAL * 60)
  {
    contadorP = 0;
    segundosAux = 0;
    etapa = PRELAV; // te dejo igual que tu “done…”
  }
}
inline void run_centrifugado(int e)
{
  if (tiempo_aux2 <= 30)
  {
    if (tiempo_aux2 >= 29)
    {
      lavado.STOP_M();
      tiempoInicio = millis(); // marcas el inicio para el “shot” de agua
    }
    else
    {
      if (tiempo_aux2 <= 5)
      {
        lavado.STOP_M();
      }
      else
      {
        lavado.DERECHA_M();
      }
    }
    return;
  }
  int dur = dur_centri(e);               // minutos
  int faltan = (dur * 60) - tiempo_aux2; // segs restantes del paso de centrifugado
  if (faltan <= 50)
  {
    lavado.STOP_M();
    return;
  }
  unsigned long ahora = millis();
  unsigned long trans = ahora - tiempoInicio;
  if (trans < tiempoDeseado)
  {
    // lavado.agua_fria_centrifugado();
  }
  else
  {
    lavado.val_off();
  }

#if amortiguador == 1
  int intervalNumber = (tiempo_aux2 / 120) % 2;
  switch (intervalNumber)
  {
  case 0:
    lavado.DERECHA_M();
    cambio = 1;
    if (cambio_2 == 1)
    {
      lavado.STOP_M();
      cambio_2 = 0;
      // tus pausas largas: mantengo la semántica sin saturar el WDT
      for (int i = 0; i < 2; i++)
      {
        WD_KICK();
        delay(6000);
      }
      for (int i = 0; i < 2; i++)
      {
        WD_KICK();
        delay(4000);
      }
    }
    break;
  case 1:
    if (cambio == 1)
    {
      lavado.STOP_M();
      cambio = 0;
      for (int i = 0; i < 2; i++)
      {
        WD_KICK();
        delay(6000);
      }
      for (int i = 0; i < 2; i++)
      {
        WD_KICK();
        delay(4000);
      }
    }
    lavado.CENTRIFUGADO();
    cambio_2 = 1;
    break;
  default:
    lavado.STOP_M();
    break;
  }
#else
  lavado.CENTRIFUGADO();
#endif
}

static inline bool paso_vacio(int etapa, int paso)
{
  if (paso == PASO_LLENA)
  {
    // LLENA solo es "vacío" si NO hay tiempo y NO hay objetivo en litros (EEPROM 200..205)
    return (dur_llenado(etapa) == 0) && (objetivo_litros_por_etapa((uint8_t)etapa) <= 0);
  }
  if (paso == PASO_DRENA)
    return dur_desague(etapa) == 0;
  if (paso == PASO_CENTRI)
    return dur_centri(etapa) == 0;
  return true;
}

inline void run_paso_actual()
{
  while (paso_vacio(etapa, paso))
  {
    lavado.STOP_M();
    reset_estado_paso();
    paso = (paso == PASO_LLENA) ? PASO_DRENA : (paso == PASO_DRENA ? PASO_CENTRI : PASO_LLENA);
    if (paso == PASO_LLENA)
      ddisplay.clear();
  }

  enviar_progreso_una_vez(etapa, paso);

  if (paso == PASO_LLENA && dur_llenado(etapa) == 0)
  {
    lavado.STOP_M();
    reset_estado_paso();
    paso = PASO_DRENA; // salta directo a drenado
    return;
  }
  if (paso == PASO_DRENA && dur_desague(etapa) == 0)
  {
    lavado.STOP_M();
    reset_estado_paso();
    paso = PASO_CENTRI; // salta a centrifugado
    return;
  }
  if (paso == PASO_CENTRI && dur_centri(etapa) == 0)
  {
    lavado.STOP_M();
    reset_estado_paso();
    paso = PASO_LLENA; // siguiente etapa arrancará llenando
    ddisplay.clear();
    return;
  }
  if (paso == PASO_LLENA)
  {
    if (tiempo_aux2 <= dur_llenado(etapa) * 60)
    {
      time = millis() / 1000;
      dato_llenado = time;
      llenado_mojado(dato_llenado, nivel_objetivo(etapa), tiempo_giro_izquierda, tiempo_giro_derecha, tiempo_reposo, tiempo_aux2, dur_llenado(etapa), temperatura, etapa);
    }
    else
    {
      lavado.STOP_M();
      reset_estado_paso();
      paso = PASO_DRENA;
    }
  }
  else if (paso == PASO_DRENA)
  {
    if (tiempo_aux2 <= dur_desague(etapa) * 60)
    {
      time = millis() / 1000;
      dato_llenado = time;
      drenado_lavado(dato_llenado, 0, tiempo_giro_izquierda, tiempo_giro_derecha, tiempo_reposo, tiempo_aux2, dur_desague(etapa));
    }
    else
    {
      lavado.STOP_M();
      reset_estado_paso();
      paso = PASO_CENTRI;
    }
  }
  else
  { // PASO_CENTRI
    if (tiempo_aux2 <= dur_centri(etapa) * 60)
    {
      run_centrifugado(etapa);
    }
    else
    {
      lavado.STOP_M();
      reset_estado_paso();
      paso = PASO_LLENA; // siguiente etapa arranca llenando
      ddisplay.clear();
    }
  }
}
inline void tick_cada_segundo()
{
  wd_mark_tick_1s();
  segundostotalB--;
  segundosAux++;
  tiempo_aux2++;
  intentar_avanzar_etapa();
}

static inline void wd_keepalive_tick_1s(uint32_t *last_ms)
{
  const uint32_t now = millis();
  if ((uint32_t)(now - *last_ms) >= 1000U)
  {
    *last_ms += 1000U;
    wd_mark_tick_1s(); // <-- habilita que WD_KICK sí resetee en ciclo
  }
  WD_KICK();
}

static inline void wd_delay_alive(uint32_t ms, uint32_t *last_ms)
{
  while (ms > 0U)
  {
    const uint16_t chunk = (ms > 50U) ? 50U : (uint16_t)ms;
    delay(chunk);
    ms -= (uint32_t)chunk;
    wd_keepalive_tick_1s(last_ms);
  }
}

bool bloqueo_puerta_loop(unsigned long &lastTime,
                         int &contador_errores_e6,
                         int aminutos,
                         volatile int &auxiliar_puerta_error)
{
  int p_continental = 5;
  uint32_t wd_last_ms = (uint32_t)millis(); // <- AQUÍ, una sola vez

  while (((digitalRead(BUTT_BOT) == 1) && (digitalRead(BUTT_TOP) == 1) && (aminutos > 0) && (auxiliar_puerta_error == 0)) ||
         ((digitalRead(BUTT_BOT) == 0) && (digitalRead(BUTT_TOP) == 1) && (aminutos > 0) && (auxiliar_puerta_error == 0)))
  {
    wd_keepalive_tick_1s(&wd_last_ms);

    if ((digitalRead(BUTT_BOT) == 0) && (digitalRead(BUTT_TOP) == 1))
    {
#if continental == 1
      int aux_rele = 0;
      while (1)
      {
        wd_keepalive_tick_1s(&wd_last_ms);

        if ((millis() - lastTime) > 1000UL)
        {
          p_continental--;
          aux_rele++;
          lastTime = millis();

          display.setBrightness(0x0f);
          display.showNumberDec(p_continental, true, 2, 2);

          if (aux_rele <= 1)
          {
            lavado.r_continental_on_1();
          }
        }

        if (p_continental <= 0)
        {
          auxiliar_puerta_error = 1;
          break;
        }
      }
#endif

      lavado.PUERTA_ON();
      ddisplay.clear();
      wd_delay_alive(1000U, &wd_last_ms); // <- en vez de delay(1000)
    }

    if ((digitalRead(BUTT_TOP) == 0) && (digitalRead(BUTT_BOT) == 0))
    {
      ddisplay.clear();
      wd_delay_alive(500U, &wd_last_ms); // <- en vez de delay(500)
      return true;
    }

    p_continental = 5;
    display.setBrightness(0x0f);
    display.setSegments(SEG_DOOR);
    wd_delay_alive(3000U, &wd_last_ms); // <- en vez de delay(3000)

    lavado.drenado();
    lavado.val_off();
    lavado.STOP_M();
    lavado.PUERTA_OFF();
    wd_delay_alive(3000U, &wd_last_ms); // <- en vez de delay(3000)

    contador_errores_e6++;
    if (contador_errores_e6 >= 20)
    {
      display.setSegments(SEG_E6);
      wd_delay_alive(3000U, &wd_last_ms); // <- en vez de delay(3000)
      enviar_evento(EV_E6);
      contador_errores_e6 = 0;
    }

    ddisplay.clear();
  }

  return false;
}

// ====== WDT helpers ======
static inline void wdt_delay_ms(uint32_t ms)
{
  while (ms > 0U)
  {
    const uint16_t chunk = (ms > 250U) ? 250U : (uint16_t)ms;
    delay(chunk);
    WD_KICK();

    ms -= (uint32_t)chunk;
  }
}

int obtenerPrimeraEtapaActiva(void)
{
  if (TIEMPO_PRELAVADO > 0)
    return 1;
  if (TIEMPO_LAVADO > 0)
    return 2;
  if (TIEMPO_ENJUAGUE > 0)
    return 3;
  if (TIEMPO_ENJUAGUE_EXTRA_1 > 0)
    return 4;
  if (TIEMPO_ENJUAGUE_EXTRA_2 > 0)
    return 5;
  if (TIEMPO_ENJUAGUE_FINAL > 0)
    return 6;
  if (TIEMPO_CENTRIFUGADO_FINAL > 0)
    return 7;
  return 0;
}

int obtenerPasoDesdeContinua(int continuaValor)
{
  switch (continuaValor)
  {
  case 12:
  case 15:
  case 18:
  case 21:
  case 24:
  case 27:
    return 1;

  case 13:
  case 16:
  case 19:
  case 22:
  case 25:
  case 28:
    return 2;

  default:
    return 0;
  }
}

void setup()
{
  wdt_disable();
  Serial.begin(9600);
  Serial2.begin(9600);
  ddisplay.clear();
  ddisplay.setBrightness(0x0f);

  debug_mostrar_reset();

  pinMode(A0, INPUT_PULLUP);
  menu_agua.cases(DEFAULT_nivel_agua, 3);
  menu_temp.cases(DEFAULT_tipo_temperatura, 2);
  menu_ciclo.cases(DEFAULT_tipo_ciclo, 1);
  seleccion_temperatura(DEFAULT_tipo_temperatura);
  seleccion_agua(DEFAULT_nivel_agua);
  seleccion_ciclo(DEFAULT_tipo_ciclo);
  pinMode(A12, OUTPUT); // B1
  pinMode(A11, OUTPUT); // B2
  pinMode(A10, OUTPUT); // B3
  pinMode(A9, OUTPUT);  // B4
  digitalWrite(A12, LOW);
  digitalWrite(A11, LOW);
  digitalWrite(A10, LOW);
  digitalWrite(A9, LOW);
  pinMode(moneda_, INPUT_PULLUP);
  pinMode(prelavadoLED, OUTPUT);
  pinMode(lavadoLED, OUTPUT);
  pinMode(enjuagueLED, OUTPUT);
  pinMode(enjuagueEXLED, OUTPUT);
  pinMode(enjugauefinLED, OUTPUT);
  pinMode(cenrifugadoLED, OUTPUT);
  digitalWrite(prelavadoLED, LOW);
  digitalWrite(lavadoLED, LOW);
  digitalWrite(enjuagueLED, LOW);
  digitalWrite(enjuagueEXLED, LOW);
  digitalWrite(enjugauefinLED, LOW);
  digitalWrite(cenrifugadoLED, LOW);
  parametros();
  ddisplay.setBrightness(0x0f);

#if (eprom == 1)
  paramreos_eeprom();
#endif
  if (tipo_ciclo.isPressed())
  {
    k = 1;
  }

#if (opl == 1)
  EEPROM.update(1001, 0); // opl por mientras
  ddisplay.clear();
  display.setBrightness(0x0f);
  display.setSegments(SEG_PAY);
#endif
#if (opl == 0)
  EEPROM.update(1001, 1); // opl por mientras
#endif

  wd_mark_tick_1s(); // deja timestamp válido
  wdt_enable(WDTO_8S);
}

void loop()
{
  switch (k)
  {

  case 1:
    WD_KICK();

    if (aux1 == 0)
    {
      ddisplay.clear();
      aux1++;
    }
    ddisplay.setBrightness(0x0f);
    display.setSegments(SEG_PROG);
    delay(3000);
    ddisplay.clear();
    display.setBrightness(0x0f);
    display.setSegments(SEG_P);
    display.showNumberDec(aux2, true, 3, 1);
    while (1)
    {
      WD_KICK();

      if (tipo_ciclo.isPressed())
      {
        aux2++;

        if (aux2 > 500)
        {
          delay(9000);
        }
        display.setBrightness(0x0f);
        display.setSegments(SEG_P);
        display.showNumberDec(aux2, true, 3, 1);
      }
      if (tipo_temperatura.isPressed())
      {
        aux2--;

        if (aux2 <= 101)
        {
          aux2 = 101;
        }
        display.setBrightness(0x0f);
        display.setSegments(SEG_P);
        display.showNumberDec(aux2, true, 3, 1);
      }
      if (nivel_agua.isPressed())
      {
        ddisplay.clear();
        int aux3 = EEPROM.read(aux2);
        display.setBrightness(0x0f);
        display.showNumberDec(aux3, true, 3, 1);
        while (1)
        {
          WD_KICK();

          if (tipo_ciclo.isPressed())
          {
            aux3++;

            if (aux3 >= 200)
            {
              aux2 = 200;
            }
            display.setBrightness(0x0f);
            display.showNumberDec(aux3, true, 3, 1);
          }
          if (tipo_temperatura.isPressed())
          {
            aux3--;
            if (aux3 <= 0)
            {
              aux3 = 0;
            }
            display.setBrightness(0x0f);
            display.showNumberDec(aux3, true, 3, 1);
          }
          if (Inicio.isPressed())
          {
            EEPROM.update(aux2, aux3);
            delay(500);
            ddisplay.clear();
            display.setBrightness(0x0f);
            display.setSegments(SEG_P);
            display.showNumberDec(aux2, true, 3, 1);
            break;
          }
        }
      }
    }
    break;
  case 0:
    WD_KICK();

    while (status_etapa_1 == 1)
    {
      unsigned long interval = 500;
      unsigned long previousMillis = millis();
      for (int i = 0; i < 1; i++)
      {
        WD_KICK();

        Serial2.print("status_2\n");
        while (millis() - previousMillis < interval)
        {
        }
        status_etapa_1 = 0;
        previousMillis = millis(); // Actualiza el tiempo para la próxima espera
      }
    }
    while (status_etapa == 1)
    {
      unsigned long interval = 500;
      unsigned long previousMillis = millis();
      for (int i = 0; i < 1; i++)
      {
        WD_KICK();

        Serial2.print("status\n");
        while (millis() - previousMillis < interval)
        {
        }
        status_etapa = 0;
        previousMillis = millis(); // Actualiza el tiempo para la próxima espera
      }
    }

    while (digitalRead(BUTT_BOT) == 0 && digitalRead(BUTT_TOP) == 0)
    {
      WD_KICK();

      display.setBrightness(0x0f);
      display.setSegments(SEG_E5);
      lavado.PUERTA_ON();
      delay(2000);
      WD_KICK();

      lavado.PUERTA_OFF();
      delay(2000);
      contador_errores_e5++;
      if (contador_errores_e5 >= 20)
      {
        enviar_evento(EV_E5);
        contador_errores_e5 = 0;
      }
      if (digitalRead(BUTT_TOP) == 1)
      {
        while (1)
          ;
      }
    }

#if serial == 1
#if opl == 1
    const int expectedLength = 10;
    char incomingData[expectedLength + 1];
    int index = 0;

    while (index < expectedLength && activacion == 0)
    {
      // Serial.print("sssssssssss");
      WD_KICK();

      if (Serial2.available() > 0)
      {
        // Lee el siguiente byte disponible
        char incomingByte = Serial.read();
        // Serial.print(incomingByte);
        index++;
      }
      /* if(EEPROM.read(33)>1){
      parametros();
      activacion = 10;
      break;
      }
      */
    }

    if (Serial2.available() > 0)
    {
      WD_KICK();

      recived = Serial2.readString();
      // Serial.print(recived);
      int startPos = recived.indexOf('{');
      int endPos = recived.indexOf('}');
      if (startPos >= 0 && endPos >= 0)
      {
        WD_KICK();

        String numbersString = recived.substring(startPos + 1, endPos);

        String numbersArray[4]; // 4 números
        int count = 0;
        char separator = ',';
        int prevIndex = 0;
        int nextIndex;
        do
        {
          WD_KICK();

          nextIndex = numbersString.indexOf(separator, prevIndex);
          if (nextIndex != -1)
          {
            numbersArray[count] = numbersString.substring(prevIndex, nextIndex);
            prevIndex = nextIndex + 1;
          }
          else
          {
            numbersArray[count] = numbersString.substring(prevIndex);
          }

          count++;
        } while (nextIndex != -1 && count < 4); // Cambia 4 al número de elementos esperados
        WD_KICK();

        if (numbersArray[0].toInt() > 0)
        {
          WD_KICK();

          parametros();
          activacion = 10;

          unsigned long interval = 1000; // Intervalo de 1 segundo entre cada envío
          unsigned long previousMillis = millis();

          for (int i = 0; i < 4; i++)
          {
            WD_KICK();

            Serial2.print("ok\n");
            // Serial2.print("ok\n"); // Envía el comando "01"
            // Serial.print("ok\n"); // Envía el comando "01"
            // Espera hasta que haya pasado 1 segundo
            while (millis() - previousMillis < interval)
            {
              // No hacer nada aquí, solo esperar
            }

            previousMillis = millis(); // Actualiza el tiempo para la próxima espera
          }

          ddisplay.clear();
          display.setBrightness(0x0f);
          display.setSegments(SEG_on);
          parametros();

          DEFAULT_tipo_ciclo = numbersArray[1].toInt();
          DEFAULT_tipo_temperatura = numbersArray[2].toInt();
          DEFAULT_nivel_agua = numbersArray[3].toInt();
          seleccion_ciclo(numbersArray[1].toInt());
          seleccion_temperatura(numbersArray[2].toInt());
          seleccion_agua(numbersArray[3].toInt());
          //// Serial.print(DEFAULT_tipo_ciclo);
          // Serial.println(DEFAULT_tipo_ciclo);
          // Serial.println(DEFAULT_tipo_temperatura);
          // Serial.println(DEFAULT_nivel_agua);
          parametros();
          // Serial.print(TIEMPO);
          // Serial.print("CCCCCCCCCCCCFVFDFGDFSDFSDFSDSDSDFSDSDCSDCSDCSDCSDCSDCSDCSDCSD");
        }
      }
      //////////////////////////////////////
      int startPos_3 = recived.indexOf('<');
      int endPos_3 = recived.indexOf('>');

      if (startPos_3 >= 0 && endPos_3 >= 0)
      {
        wdt_reset();
        String numbersString = recived.substring(startPos_3 + 1, endPos_3);

        String numbersArray[4];
        int count = 0;
        char separator = ',';
        int prevIndex = 0;
        int nextIndex;

        do
        {
          wdt_reset();
          nextIndex = numbersString.indexOf(separator, prevIndex);

          if (nextIndex != -1)
          {
            numbersArray[count] = numbersString.substring(prevIndex, nextIndex);
            prevIndex = nextIndex + 1;
          }
          else
          {
            numbersArray[count] = numbersString.substring(prevIndex);
          }

          count++;
        } while (nextIndex != -1 && count < 4);

        if (numbersArray[0].toInt() > 0)
        {
          activacion_2 = 10; // hay ciclo pendiente
        }
        else
        {
          activacion_2 = 0; // no hay ciclo pendiente
          etapa_2 = 0;
          continua = 0;
        }
      }
      //////////////////////////////////////
      int startPos_1 = recived.indexOf('[');
      int endPos_1 = recived.indexOf(']');

      if (startPos_1 >= 0 && endPos_1 >= 0)
      {
        wdt_reset();
        String numbersString = recived.substring(startPos_1 + 1, endPos_1);

        String numbersArray[4];
        int count = 0;
        char separator = ',';
        int prevIndex = 0;
        int nextIndex;

        do
        {
          wdt_reset();
          nextIndex = numbersString.indexOf(separator, prevIndex);

          if (nextIndex != -1)
          {
            numbersArray[count] = numbersString.substring(prevIndex, nextIndex);
            prevIndex = nextIndex + 1;
          }
          else
          {
            numbersArray[count] = numbersString.substring(prevIndex);
          }

          count++;
        } while (nextIndex != -1 && count < 4);

        etapa_2 = (numbersArray[0].toInt()) - 2;
        continua = numbersArray[3].toInt();

        if (etapa_2 <= 0)
        {
          etapa_2 = obtenerPrimeraEtapaActiva();
        }

        if (etapa_2 > 7)
        {
          etapa_2 = 7;
        }

        // Solo autoriza reanudación real si ya había pendiente
        if ((activacion_2 == 10) && (etapa_2 > 0))
        {
          activacion = 10;
        }

        // Serial.print("etapa_2: ");
        // Serial.println(etapa_2);

        // Serial.print("continua: ");
        //  Serial.println(continua);
      }
      ////////
    }
#endif
#endif
#if precio_on == 3

    if (DEFAULT_tipo_ciclo == 1)
    {
      // precio_ciclo = EEPROM.read(500);
      precio_ciclo = ciclo_1;
    }
    if (DEFAULT_tipo_ciclo == 2)
    {
      // precio_ciclo = EEPROM.read(501);
      precio_ciclo = ciclo_2;
    }
    if (DEFAULT_tipo_ciclo == 3)
    {
      // precio_ciclo = EEPROM.read(502);
      precio_ciclo = ciclo_3;
    }
    if (DEFAULT_tipo_ciclo == 4)
    {
      // precio_ciclo = EEPROM.read(502);
      precio_ciclo = ciclo_4;
    }

    if (DEFAULT_tipo_temperatura == 1)
    {
      precio_temp = 0;
    }
    if (DEFAULT_tipo_temperatura == 2)
    {
      // precio_temp = EEPROM.read(503);
      precio_temp = precio_tibia;
    }
    if (DEFAULT_tipo_temperatura == 3)
    {
      // precio_temp = EEPROM.read(504);
      precio_temp = precio_caliente;
    }
    if (DEFAULT_nivel_agua == 1)
    {
      precio_nivel = 0;
    }

    if (DEFAULT_nivel_agua == 2)
    {
      precio_nivel = nivel_2;
    }
    if (DEFAULT_nivel_agua == 3)
    {
      precio_nivel = nivel_3;
    }
    if (activacion != 10)
    {
      ddisplay.clear();
      display.setBrightness(0x0f);
      display.showNumberDec(((precio_temp + precio_ciclo + precio_nivel) * valor_moneda), true, 3, 1);
    }
    else
    {
      ddisplay.clear();
      display.setBrightness(0x0f);
      display.setSegments(SEG_on);
    }

    // Expect: __04

    while (activacion != 10)
    {
      WD_KICK();

      if (Inicio.isPressed()) // evaluo si el boton fue presionado
      {
        if ((millis() - lastDebounceTime) > debounceDelay)
        {
          while (1)
          {
          }
        }
        if (DEFAULT_tipo_ciclo == 1)
        {
          // precio_ciclo = EEPROM.read(500);
          precio_ciclo = ciclo_1;
        }
        if (DEFAULT_tipo_ciclo == 2)
        {
          // precio_ciclo = EEPROM.read(501);
          precio_ciclo = ciclo_2;
        }
        if (DEFAULT_tipo_ciclo == 3)
        {
          // precio_ciclo = EEPROM.read(502);
          precio_ciclo = ciclo_3;
        }
        if (DEFAULT_tipo_ciclo == 4)
        {
          // precio_ciclo = EEPROM.read(502);
          precio_ciclo = ciclo_4;
        }
        if (DEFAULT_tipo_temperatura == 1)
        {
          precio_temp = 0;
        }
        if (DEFAULT_tipo_temperatura == 2)
        {
          // precio_temp = EEPROM.read(503);
          precio_temp = precio_tibia;
        }
        if (DEFAULT_tipo_temperatura == 3)
        {
          // precio_temp = EEPROM.read(504);
          precio_temp = precio_caliente;
        }
        if (DEFAULT_nivel_agua == 1)
        {
          precio_nivel = 0;
        }

        if (DEFAULT_nivel_agua == 2)
        {
          precio_nivel = nivel_2;
        }
        if (DEFAULT_nivel_agua == 3)
        {
          precio_nivel = nivel_3;
        }
        ddisplay.clear();
        display.setBrightness(0x0f);
        display.showNumberDec(((precio_temp + precio_ciclo + precio_nivel) * valor_moneda), true, 3, 1); // Expect: __04
      }

      if (tipo_ciclo.isPressed()) // evaluo si el boton fue presionado
      {
        if (dato_moneda == HIGH && datoAnterior_moneda == LOW)
        {
          contador_moneda++;
        }
        datoAnterior_moneda = dato_moneda;
        DEFAULT_tipo_ciclo++;
        DEFAULT_tipo_ciclo = tipo_ciclo.pulses(DEFAULT_tipo_ciclo); // limita los estados que puede haber en el switch
        menu_ciclo.cases(DEFAULT_tipo_ciclo, 1);
        seleccion_ciclo(DEFAULT_tipo_ciclo);
        seleccion_temperatura(DEFAULT_tipo_temperatura);
        seleccion_agua(DEFAULT_nivel_agua);
        led.ciclo_LED();
        parametros();

        if (DEFAULT_tipo_ciclo == 1)
        {
          // precio_ciclo = EEPROM.read(500);
          precio_ciclo = ciclo_1;
        }
        if (DEFAULT_tipo_ciclo == 2)
        {
          // precio_ciclo = EEPROM.read(501);
          precio_ciclo = ciclo_2;
        }
        if (DEFAULT_tipo_ciclo == 3)
        {
          // precio_ciclo = EEPROM.read(502);
          precio_ciclo = ciclo_3;
        }
        if (DEFAULT_tipo_ciclo == 4)
        {
          // precio_ciclo = EEPROM.read(502);
          precio_ciclo = ciclo_4;
        }
        if (DEFAULT_tipo_temperatura == 1)
        {
          precio_temp = 0;
        }
        if (DEFAULT_tipo_temperatura == 2)
        {
          // precio_temp = EEPROM.read(503);
          precio_temp = precio_tibia;
        }
        if (DEFAULT_tipo_temperatura == 3)
        {
          // precio_temp = EEPROM.read(504);
          precio_temp = precio_caliente;
        }
        if (DEFAULT_nivel_agua == 1)
        {
          precio_nivel = 0;
        }

        if (DEFAULT_nivel_agua == 2)
        {
          precio_nivel = nivel_2;
        }
        if (DEFAULT_nivel_agua == 3)
        {
          precio_nivel = nivel_3;
        }
        ddisplay.clear();
        display.setBrightness(0x0f);
        display.showNumberDec(((precio_temp + precio_ciclo + precio_nivel) * valor_moneda) - (contador_moneda * valor_moneda), true, 3, 1);
      }

      if (tipo_temperatura.isPressed()) // evaluo si el boton fue presionado
      {
        if (dato_moneda == HIGH && datoAnterior_moneda == LOW)
        {
          contador_moneda++;
        }
        datoAnterior_moneda = dato_moneda;
        DEFAULT_tipo_temperatura++;
        DEFAULT_tipo_temperatura = tipo_temperatura.pulses(DEFAULT_tipo_temperatura); // limita los estados que puede haber en el switch
        menu_temp.cases(DEFAULT_tipo_temperatura, 2);
        seleccion_ciclo(DEFAULT_tipo_ciclo);
        seleccion_temperatura(DEFAULT_tipo_temperatura);
        seleccion_agua(DEFAULT_nivel_agua);
        led.temperatura_LED();
        parametros();

        if (DEFAULT_tipo_ciclo == 1)
        {
          // precio_ciclo = EEPROM.read(500);
          precio_ciclo = ciclo_1;
        }
        if (DEFAULT_tipo_ciclo == 2)
        {
          // precio_ciclo = EEPROM.read(501);
          precio_ciclo = ciclo_2;
        }
        if (DEFAULT_tipo_ciclo == 3)
        {
          // precio_ciclo = EEPROM.read(502);
          precio_ciclo = ciclo_3;
        }
        if (DEFAULT_tipo_ciclo == 4)
        {
          // precio_ciclo = EEPROM.read(502);
          precio_ciclo = ciclo_4;
        }
        if (DEFAULT_tipo_temperatura == 1)
        {
          precio_temp = 0;
        }
        if (DEFAULT_tipo_temperatura == 2)
        {
          // precio_temp = EEPROM.read(503);
          precio_temp = precio_tibia;
        }
        if (DEFAULT_tipo_temperatura == 3)
        {
          // precio_temp = EEPROM.read(504);
          precio_temp = precio_caliente;
        }
        if (DEFAULT_nivel_agua == 1)
        {
          precio_nivel = 0;
        }

        if (DEFAULT_nivel_agua == 2)
        {
          precio_nivel = nivel_2;
        }
        if (DEFAULT_nivel_agua == 3)
        {
          precio_nivel = nivel_3;
        }
        ddisplay.clear();
        display.setBrightness(0x0f);
        display.showNumberDec(((precio_temp + precio_ciclo + precio_nivel) * valor_moneda) - (contador_moneda * valor_moneda), true, 3, 1);
      }

      if (nivel_agua.isPressed()) // evaluo si el boton fue presionado
      {
        dato_moneda = digitalRead(moneda_);
        // revisar
        if (dato_moneda == HIGH && datoAnterior_moneda == LOW)
        {
          contador_moneda++;
        }
        datoAnterior_moneda = dato_moneda;

        DEFAULT_nivel_agua++;
        DEFAULT_nivel_agua = nivel_agua.pulses(DEFAULT_nivel_agua); // limita los estados que puede haber en el switch
        menu_agua.cases(DEFAULT_nivel_agua, 3);
        seleccion_ciclo(DEFAULT_tipo_ciclo);
        seleccion_temperatura(DEFAULT_tipo_temperatura);
        seleccion_agua(DEFAULT_nivel_agua);
        led.agua_LED();
        parametros();

        if (DEFAULT_tipo_ciclo == 1)
        {
          // precio_ciclo = EEPROM.read(500);
          precio_ciclo = ciclo_1;
        }
        if (DEFAULT_tipo_ciclo == 2)
        {
          // precio_ciclo = EEPROM.read(501);
          precio_ciclo = ciclo_2;
        }
        if (DEFAULT_tipo_ciclo == 3)
        {
          // precio_ciclo = EEPROM.read(502);
          precio_ciclo = ciclo_3;
        }
        if (DEFAULT_tipo_ciclo == 4)
        {
          // precio_ciclo = EEPROM.read(502);
          precio_ciclo = ciclo_4;
        }
        if (DEFAULT_tipo_temperatura == 1)
        {
          precio_temp = 0;
        }
        if (DEFAULT_tipo_temperatura == 2)
        {
          // precio_temp = EEPROM.read(503);
          precio_temp = precio_tibia;
        }
        if (DEFAULT_tipo_temperatura == 3)
        {
          // precio_temp = EEPROM.read(504);
          precio_temp = precio_caliente;
        }
        if (DEFAULT_nivel_agua == 1)
        {
          precio_nivel = 0;
        }

        if (DEFAULT_nivel_agua == 2)
        {
          precio_nivel = nivel_2;
        }
        if (DEFAULT_nivel_agua == 3)
        {
          precio_nivel = nivel_3;
        }
        ddisplay.clear();
        display.setBrightness(0x0f);
        display.showNumberDec(((precio_temp + precio_ciclo + precio_nivel) * valor_moneda) - (contador_moneda * valor_moneda), true, 3, 1);
      }
      dato_moneda = digitalRead(moneda_);
      // revisar
      if (dato_moneda == HIGH && datoAnterior_moneda == LOW)
      {
        lastDebounceTime = millis();
        debounceDelay = 300000;
        contador_moneda++;
        tone(buzzer, 1000);
        delay(100);
        noTone(buzzer);
        ddisplay.clear();
        display.setBrightness(0x0f);
        display.showNumberDec(((precio_temp + precio_ciclo + precio_nivel) * valor_moneda) - (contador_moneda * valor_moneda), true, 3, 1);
        if (((precio_temp + precio_ciclo + precio_nivel) * valor_moneda) <= contador_moneda * valor_moneda)
        {
          contador_moneda = 0;
          activacion = 10;
          break;
        }
      }
      datoAnterior_moneda = dato_moneda;
    }
#endif
    if (activacion == 0)
    {
      WD_KICK();

      if (tipo_ciclo.isPressed()) // evaluo si el boton fue presionado
      {
        WD_KICK();

        if (dato_moneda == HIGH && datoAnterior_moneda == LOW)
        {
          contador_moneda++;
        }
        datoAnterior_moneda = dato_moneda;
        DEFAULT_tipo_ciclo++;
        DEFAULT_tipo_ciclo = tipo_ciclo.pulses(DEFAULT_tipo_ciclo); // limita los estados que puede haber en el switch
        menu_ciclo.cases(DEFAULT_tipo_ciclo, 1);
        seleccion_ciclo(DEFAULT_tipo_ciclo);
        seleccion_temperatura(DEFAULT_tipo_temperatura);
        seleccion_agua(DEFAULT_nivel_agua);
        led.ciclo_LED();
        parametros();
      }
      if (tipo_temperatura.isPressed()) // evaluo si el boton fue presionado
      {
        WD_KICK();

        if (dato_moneda == HIGH && datoAnterior_moneda == LOW)
        {
          contador_moneda++;
        }
        datoAnterior_moneda = dato_moneda;
        DEFAULT_tipo_temperatura++;
        if (DEFAULT_tipo_temperatura > 3)
        {
          DEFAULT_tipo_temperatura = 1;
        }
        DEFAULT_tipo_temperatura = tipo_temperatura.pulses(DEFAULT_tipo_temperatura); // limita los estados que puede haber en el switch
        menu_temp.cases(DEFAULT_tipo_temperatura, 2);
        seleccion_ciclo(DEFAULT_tipo_ciclo);             // carga de la eeprom los tiempos
        seleccion_temperatura(DEFAULT_tipo_temperatura); // carga de la eeprom el tipo de temperatura
        seleccion_agua(DEFAULT_nivel_agua);              // carga de la eeprom el nivel de agua
        led.temperatura_LED();
        parametros();
      }
      if (nivel_agua.isPressed()) // evaluo si el boton fue presionado
      {
        WD_KICK();

        if (dato_moneda == HIGH && datoAnterior_moneda == LOW)
        {
          contador_moneda++;
        }
        datoAnterior_moneda = dato_moneda;
        DEFAULT_nivel_agua++;
        if (DEFAULT_nivel_agua > 3)
        {
          DEFAULT_nivel_agua = 1;
        }
        DEFAULT_nivel_agua = nivel_agua.pulses(DEFAULT_nivel_agua); // limita los estados que puede haber en el switch
        menu_agua.cases(DEFAULT_nivel_agua, 3);
        seleccion_ciclo(DEFAULT_tipo_ciclo);
        seleccion_temperatura(DEFAULT_tipo_temperatura);
        seleccion_agua(DEFAULT_nivel_agua);
        led.agua_LED();
        parametros();
      }
    }
    //    if (Inicio.isPressed() || etapa_2 > 2 || activacion_2 == 10) // evaluo si el boton fue presionado

    bool botonInicio = Inicio.isPressed();
    bool oplLista = (EEPROM.read(1001) == 1);

    // Reanudar automático: hubo ciclo pendiente y ya tengo etapa válida
    bool reanudarAhora = (activacion_2 == 10) && (etapa_2 > 0);

    // Inicio normal: requiere botón + activación normal (o modo OPL), pero NO reanudación
    bool iniciarAhora = (botonInicio == true) && ((activacion == 10) || oplLista) && (reanudarAhora == false);

    if (iniciarAhora || reanudarAhora)
    {
      if (EEPROM.read(102) == 1)
      {
        ciclo_str = DEFAULT_tipo_ciclo;
        etapa_str = 0; /////0
        paso_str = 51; /////0
        String estado_lavadora = "{" + String(ciclo_str) + "," + String(etapa_str) + "," + String(paso_str) + "}\n";
        Serial2.print(estado_lavadora);
        EEPROM.update(102, 0);
        while (1)
        {
          Serial2.print("ok_2\n");
          digitalWrite(prelavadoLED, HIGH);
          digitalWrite(lavadoLED, HIGH);
          digitalWrite(enjuagueLED, HIGH);
          digitalWrite(enjuagueEXLED, HIGH);
          digitalWrite(enjugauefinLED, HIGH);
          digitalWrite(cenrifugadoLED, HIGH);
        }
      }

      // ===== AQUÍ decides desde dónde arrancar =====
      if (reanudarAhora)
      {
        etapa = etapa_2;
        paso = obtenerPasoDesdeContinua(continua);

        // Reanudas al INICIO del paso actual, no a mitad del paso
        tiempo_aux2 = 0;

        if (paso == PASO_LLENA)
        {
          segundosAux = 0;
        }
        else if (paso == PASO_DRENA)
        {
          segundosAux = (unsigned long)dur_llenado(etapa) * 60UL;
        }
        else // PASO_CENTRI
        {
          segundosAux = (unsigned long)(dur_llenado(etapa) + dur_desague(etapa)) * 60UL;
        }

        activacion_2 = 0; // consumir reanudación
        activacion = 0;   // limpiar activación vieja, ya no la necesitas aquí
      }
      else
      {
        etapa = obtenerPrimeraEtapaActiva();
        paso = PASO_LLENA;
        segundosAux = 0;
        tiempo_aux2 = 0;
        etapa_2 = 0;
        continua = 0;
      }

      WD_KICK();

      // if (activacion == 10 || EEPROM.read(1001) == 1)
      if (reanudarAhora || activacion == 10 || EEPROM.read(1001) == 1)
      {

        WD_KICK();

#if opl == 0
        if (TIEMPO_PRELAVADO * 60 > 0)
          etapa = PRELAV;
        else if (TIEMPO_PRELAVADO * 60 == 0)
          etapa = LAV;
        else if (TIEMPO_LAVADO * 60 == 0)
          etapa = ENJ;
        else if (TIEMPO_ENJUAGUE * 60 == 0)
          etapa = ENJX1;
        else if (TIEMPO_ENJUAGUE_EXTRA_1 * 60 == 0)
          etapa = ENJX2;
        else if (TIEMPO_ENJUAGUE_EXTRA_2 * 60 == 0)
          etapa = ENJF;
        else if (TIEMPO_ENJUAGUE_FINAL * 60 == 0)
          etapa = SPINF;
#endif
        segundostotal = TIEMPO * 60;
        ahoras = ((segundostotal / 60) / 60);
        aminutos = (segundostotal / 60) % 60;
        asegundos = segundostotal % 60;
        segundostotal = asegundos + (aminutos * 60) + (ahoras * 60 * 60);

        if (segundostotal == 0)
        {
          etapa = 0;
          delay(1000);
          while (1)
          {
          }
        }
        if (etapa == PRELAV)
          segundostotalB = segundostotal;
        if (etapa == LAV)
          segundostotalB = segundostotal - TIEMPO_PRELAVADO * 60;
        if (etapa == ENJ)
          segundostotalB = segundostotal - (TIEMPO_PRELAVADO + TIEMPO_LAVADO) * 60;
        if (etapa == ENJX1)
          segundostotalB = segundostotal - (TIEMPO_PRELAVADO + TIEMPO_LAVADO + TIEMPO_ENJUAGUE) * 60;
        if (etapa == ENJX2)
          segundostotalB = segundostotal - (TIEMPO_PRELAVADO + TIEMPO_LAVADO + TIEMPO_ENJUAGUE + TIEMPO_ENJUAGUE_EXTRA_1) * 60;
        if (etapa == ENJF)
          segundostotalB = segundostotal - (TIEMPO_PRELAVADO + TIEMPO_LAVADO + TIEMPO_ENJUAGUE + TIEMPO_ENJUAGUE_EXTRA_1 + TIEMPO_ENJUAGUE_EXTRA_2) * 60;
        if (etapa == SPINF)
          segundostotalB = segundostotal - (TIEMPO_PRELAVADO + TIEMPO_LAVADO + TIEMPO_ENJUAGUE + TIEMPO_ENJUAGUE_EXTRA_1 + TIEMPO_ENJUAGUE_EXTRA_2 + TIEMPO_ENJUAGUE_FINAL) * 60;

        if (reanudarAhora)
        {
          segundostotalB -= (long)segundosAux;
          if (segundostotalB < 0)
          {
            segundostotalB = 0;
          }
        }

        if (!instruccionEnviada29)
        {
          enviar_evento(EV_INICIO);
          instruccionEnviada29 = true;
        }
        __clave_progreso_prev = -1;

        ddisplay.clear();
        led.inicio_LED();
        medidor_reset_remote();
        ciclo_begin_agua();
        especiales_reset();
        while (segundostotalB > 0)
        {

          WD_KICK();

          if (!bloqueo_puerta_loop(lastTime, contador_errores_e6, aminutos, auxiliar_puerta_error))
          {
          }
          time = millis() / 1000;
          dato2 = time;
          if (dato2 != datoAnterior2)
          {
            tick_cada_segundo();
            datoAnterior2 = dato2;
          }
          ahoras = ((segundostotalB / 60) / 60);
          aminutos = (segundostotalB / 60) % 60;
          asegundos = segundostotalB % 60;

          if (aminutos > 0)
          {
            display.setBrightness(0x0f);
            uint8_t data[4];
            display.showNumberDec(aminutos, true, 2, 2);
            data[0] = 0x00;
            data[1] = 0x00;
            data[2] = display.encodeDigit(aminutos / 10);
            data[3] = display.encodeDigit(aminutos % 10);
            display.setSegments(data);

            if (aminutos < 8)
            {
#if continental == 1
              lavado.r_continental_off_1();
              lavado.PUERTA_OFF();
#endif
            }
          }
          else
          {
            display.setBrightness(0x0f);
            display.showNumberDec(asegundos, true, 2, 2);
            if (asegundos == 0)
            {
              lavado.PUERTA_OFF();
              ciclo_str = DEFAULT_tipo_ciclo;
              etapa_str = 0; /////0
              paso_str = 51; /////0
              String estado_lavadora = "{" + String(ciclo_str) + "," + String(etapa_str) + "," + String(paso_str) + "}\n";
              Serial2.print(estado_lavadora);
              delay(500);

              unsigned long interval = 1000; // Intervalo de 1 segundo entre cada envío
              unsigned long previousMillis = millis();
              for (int i = 0; i < 4; i++)
              {
                WD_KICK();
                Serial2.print("ok_2\n");

                while (millis() - previousMillis < interval)
                {
                  // No hacer nada aquí, solo esperar
                }
                previousMillis = millis(); // Actualiza el tiempo para la próxima espera
              }

              unsigned long Lfinal = ciclo_total_agua_L();
              enviar_evento(EVT_FIN);

              ciclo_end_agua();
              mostrar_total_ciclo_display(Lfinal);
              while (1)
              {
                WD_KICK();

                time = millis() / 1000;
                datoP = time;
                if (datoP != datoAnteriorP)
                {
                  ddisplay.clear();
                  display.setBrightness(0x0f);
                  display.setSegments(SEG_DONE);
                  contadorP++;
                  if (contadorP % 2 == 0)
                    noTone(buzzer);
                  else
                    tone(buzzer, 2000);
                  if (contadorP >= 10)
                  {
                    digitalWrite(prelavadoLED, LOW);
                    digitalWrite(lavadoLED, LOW);
                    digitalWrite(enjuagueLED, LOW);
                    digitalWrite(enjuagueEXLED, LOW);
                    digitalWrite(enjugauefinLED, LOW);
                    digitalWrite(cenrifugadoLED, LOW);
                    activacion = 0;
                    etapa = LAV; // o 1/2 según tu preferencia actual; dejé igual que tu caso
                    ciclo_str = DEFAULT_tipo_ciclo;
                    etapa_str = 0; /////0
                    paso_str = 51; /////0
                    String estado_lavadora = "{" + String(ciclo_str) + "," + String(etapa_str) + "," + String(paso_str) + "}\n";
                    Serial2.print(estado_lavadora);
                    delay(500);

                    unsigned long interval = 1000; // Intervalo de 1 segundo entre cada envío
                    unsigned long previousMillis = millis();
                    for (int i = 0; i < 4; i++)
                    {
                      WD_KICK();
                      Serial2.print("ok_2\n");

                      while (millis() - previousMillis < interval)
                      {
                        // No hacer nada aquí, solo esperar
                      }
                      previousMillis = millis(); // Actualiza el tiempo para la próxima espera
                    }

                    enviar_evento(EVT_FIN);
                    while (1)
                    {
                    }
                  }
                }
                datoAnteriorP = datoP;
              }
            }
          }
          digitalWrite(prelavadoLED, etapa == PRELAV ? HIGH : LOW);
          digitalWrite(lavadoLED, etapa == LAV ? HIGH : LOW);
          digitalWrite(enjuagueLED, etapa == ENJ ? HIGH : LOW);
          digitalWrite(enjuagueEXLED, (etapa == ENJX1 || etapa == ENJX2) ? HIGH : LOW);
          digitalWrite(enjugauefinLED, etapa == ENJF ? HIGH : LOW);
          digitalWrite(cenrifugadoLED, etapa == SPINF ? HIGH : LOW);
          run_paso_actual();
        }
      }
    }

    break;
  }
}