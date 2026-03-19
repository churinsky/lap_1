
#ifndef _LAVADO_H_
#define _LAVADO_H_

// para la tarjeta anterior

#if modelo == 1 // anterior
#include <Arduino.h>
class Lavado
{
#define buzzer 13
    int _PRESOSTATO;                // = A0;
    int _MOTOR_BAJA;                //= 34; //motor baja   EL 34 ES CORRECTO AQUI se cambnio a 33 en peña
    int _SECUENCIA;                 //= 39;   //cambio de secuencia
    int _MOTOR_ALTA;                //= 28;  //motor alta
    int _PUERTA;                    //= 22;
    int _DESAGUE;                   //= 26;
    int _TAMBOR_DEPOSITO1_FRIA;     //= 36;
    int _TAMBOR_DEPOSITO1_CALIENTE; // = 44;
    int _DEPOSITO2;                 //= 23;
    int _DEPOSITO3;                 //= 24;
    int _BUTT_TOP;                  //= 30; //button top-------------FALTA DEFINIRLOS
    int _BUTT_BOT;                  //= A2; //button bottom------------- FALTA DEFINIRLOS
    int _PUERTA_ON_110;             // MISO
    int _PUERTA_OFF_110;            // MOSI
    int _CONTROL_VAC;               // SCK
public:
    Lavado(int presostato, int motor_baja, int secuencia, int motor_alta, int puerta, int desague, int tambor_deposito1_fria, int tambor_deposito1_caliente, int deposito2, int deposito3, int but_top, int but_bot, int puerta_on_100, int puerta_off_100, int control_vac)
    {
        _PRESOSTATO = presostato;
        _MOTOR_BAJA = motor_baja;
        _SECUENCIA = secuencia;
        _MOTOR_ALTA = motor_alta;
        _PUERTA = puerta;
        _DESAGUE = desague;
        _TAMBOR_DEPOSITO1_FRIA = tambor_deposito1_fria;
        _TAMBOR_DEPOSITO1_CALIENTE = tambor_deposito1_caliente;
        _DEPOSITO2 = deposito2;
        _DEPOSITO3 = deposito3;
        _BUTT_TOP = but_top;
        _BUTT_BOT = but_bot;
        _PUERTA_ON_110 = puerta_on_100;   // MISO
        _PUERTA_OFF_110 = puerta_off_100; // MOSI
        _CONTROL_VAC = control_vac;       // SCK

        //////////////////////////

        pinMode(_PUERTA_ON_110, OUTPUT);  // miso
        pinMode(_PUERTA_OFF_110, OUTPUT); // mosi
        pinMode(_CONTROL_VAC, OUTPUT);    // sck

        digitalWrite(_PUERTA_ON_110, HIGH); // LOW
        digitalWrite(_PUERTA_OFF_110, HIGH);
        digitalWrite(_CONTROL_VAC, HIGH);
        //////////////////////////

        pinMode(_MOTOR_BAJA, OUTPUT);
        pinMode(_MOTOR_ALTA, OUTPUT);
        pinMode(_SECUENCIA, OUTPUT);
        pinMode(_PUERTA, OUTPUT);
        pinMode(_DESAGUE, OUTPUT);
        pinMode(_TAMBOR_DEPOSITO1_FRIA, OUTPUT);
        pinMode(_TAMBOR_DEPOSITO1_CALIENTE, OUTPUT);
        pinMode(_DEPOSITO2, OUTPUT);
        pinMode(_DEPOSITO3, OUTPUT);
        pinMode(_BUTT_TOP, INPUT_PULLUP);
        pinMode(_BUTT_BOT, INPUT_PULLUP);

        digitalWrite(_MOTOR_BAJA, HIGH);
        digitalWrite(_MOTOR_ALTA, HIGH);
        digitalWrite(_SECUENCIA, HIGH);
        digitalWrite(_PUERTA, HIGH);
        digitalWrite(_DESAGUE, HIGH);
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, HIGH);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, HIGH);
        digitalWrite(_DEPOSITO2, HIGH);
        digitalWrite(_DEPOSITO3, HIGH);
    }

    void STOP_M()
    {
        digitalWrite(_MOTOR_BAJA, HIGH);
        digitalWrite(_MOTOR_ALTA, HIGH);
        digitalWrite(_SECUENCIA, HIGH);
    }

    void val_off()
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, HIGH);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, HIGH);
        digitalWrite(_DEPOSITO2, HIGH);
        digitalWrite(_DEPOSITO3, HIGH);
    }

    void CENTRIFUGADO()
    {
        drenado();
        digitalWrite(_SECUENCIA, HIGH);
        digitalWrite(_MOTOR_BAJA, HIGH);
        digitalWrite(_MOTOR_ALTA, LOW);
    }
    void drenado()
    {
        digitalWrite(_DESAGUE, HIGH);
    }

    void no_drenado()
    {
        digitalWrite(_DESAGUE, LOW);
    }
    void PUERTA_OFF()
    {
        /////////////////////////////
        digitalWrite(_CONTROL_VAC, HIGH);
        delay(500);
        digitalWrite(_PUERTA_OFF_110, LOW);
        delay(500);
        digitalWrite(_PUERTA_OFF_110, HIGH);
        delay(500);
        //////////////////////////////
        digitalWrite(_PUERTA, HIGH);
    }

    void PUERTA_ON()
    {
        digitalWrite(_PUERTA, LOW);
        ////////////////////////////
        delay(500);
        digitalWrite(_CONTROL_VAC, LOW);
        delay(500);
        digitalWrite(_PUERTA_ON_110, LOW);
        delay(500);
        digitalWrite(_PUERTA_ON_110, HIGH);
        ////////////////////////////
    }

    void IZQUIERDA_M()
    {
        digitalWrite(_SECUENCIA, LOW);
        digitalWrite(_MOTOR_BAJA, HIGH);
        digitalWrite(_MOTOR_ALTA, HIGH);
    }
    void DERECHA_M()
    {
        digitalWrite(_SECUENCIA, HIGH);
        digitalWrite(_MOTOR_BAJA, LOW);
        digitalWrite(_MOTOR_ALTA, HIGH);
    }

    void agua_fria_centrifugado() // lavado
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, LOW);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, HIGH);
        digitalWrite(_DEPOSITO2, LOW);
        digitalWrite(_DEPOSITO3, HIGH);
    }

    void agua_fria() // lavado
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, LOW);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, HIGH);
        digitalWrite(_DEPOSITO2, LOW);
        digitalWrite(_DEPOSITO3, HIGH);
        no_drenado();
    }
    void agua_tibia() // lavado
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, LOW);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, LOW);
        digitalWrite(_DEPOSITO2, LOW);
        digitalWrite(_DEPOSITO3, HIGH);
        no_drenado();
    }
    void agua_caliente() // lavado
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, HIGH);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, LOW);
        digitalWrite(_DEPOSITO2, LOW);
        digitalWrite(_DEPOSITO3, HIGH);
        no_drenado();
    }
    void enjuague_final() // enjuague final
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, LOW);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, HIGH);
        digitalWrite(_DEPOSITO2, LOW);
        digitalWrite(_DEPOSITO3, LOW);
        no_drenado();
    }

    void agua_fria_enjuagues() // enjuagues
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, LOW);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, HIGH);
        digitalWrite(_DEPOSITO2, LOW);
        digitalWrite(_DEPOSITO3, HIGH);
        no_drenado();
    }
    void agua_tibia_enjuagues() // enjuagues
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, LOW);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, LOW);
        digitalWrite(_DEPOSITO2, LOW);
        digitalWrite(_DEPOSITO3, HIGH);
        no_drenado();
    }
    void agua_caliente_enjuagues() // enjuagues
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, HIGH);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, LOW);
        digitalWrite(_DEPOSITO2, LOW);
        digitalWrite(_DEPOSITO3, HIGH);
        no_drenado();
    }
};
#elif modelo == 0 // nuevo
#include <Arduino.h>

class Lavado
{
#define buzzer 13
    int _PRESOSTATO;                // = A0;
    int _MOTOR_BAJA;                //= 34; //motor baja   EL 34 ES CORRECTO AQUI
    int _SECUENCIA;                 //= 39;   //cambio de secuencia
    int _MOTOR_ALTA;                //= 28;  //motor alta
    int _PUERTA;                    //= 22;
    int _DESAGUE;                   //= 26;
    int _TAMBOR_DEPOSITO1_FRIA;     //= 36;
    int _TAMBOR_DEPOSITO1_CALIENTE; // = 44;
    int _DEPOSITO2;                 //= 23;
    int _DEPOSITO3;                 //= 24;
    int _BUTT_TOP;                  //= 30; //button top-------------FALTA DEFINIRLOS
    int _BUTT_BOT;                  //= A2; //button bottom------------- FALTA DEFINIRLOS
    int _PUERTA_ON_110;             // MISO
    int _PUERTA_OFF_110;            // MOSI
    int _CONTROL_VAC;               // SCK
public:
    Lavado(int presostato, int motor_baja, int secuencia, int motor_alta, int puerta, int desague, int tambor_deposito1_fria, int tambor_deposito1_caliente, int deposito2, int deposito3, int but_top, int but_bot, int puerta_on_100, int puerta_off_100, int control_vac)
    {
        _PRESOSTATO = presostato;
        _MOTOR_BAJA = motor_baja;
        _SECUENCIA = secuencia;
        _MOTOR_ALTA = motor_alta;
        _PUERTA = puerta;
        _DESAGUE = desague;
        _TAMBOR_DEPOSITO1_FRIA = tambor_deposito1_fria;
        _TAMBOR_DEPOSITO1_CALIENTE = tambor_deposito1_caliente;
        _DEPOSITO2 = deposito2;
        _DEPOSITO3 = deposito3;
        _BUTT_TOP = but_top;
        _BUTT_BOT = but_bot;
        _PUERTA_ON_110 = puerta_on_100;   // MISO
        _PUERTA_OFF_110 = puerta_off_100; // MOSI
        _CONTROL_VAC = control_vac;       // SCK

        //////////////////////////

        pinMode(_PUERTA_ON_110, OUTPUT);  // miso
        pinMode(_PUERTA_OFF_110, OUTPUT); // mosi
        pinMode(_CONTROL_VAC, OUTPUT);    // sck
#if activacion_puerta == 0

        digitalWrite(_PUERTA_ON_110, LOW); // LOW
        digitalWrite(_PUERTA_OFF_110, LOW);
        digitalWrite(_CONTROL_VAC, LOW);
#endif
#if activacion_puerta == 1

        digitalWrite(_PUERTA_ON_110, HIGH); // LOW
        digitalWrite(_PUERTA_OFF_110, HIGH);
        digitalWrite(_CONTROL_VAC, HIGH);
#endif
        //////////////////////////

        pinMode(_MOTOR_BAJA, OUTPUT);
        pinMode(_MOTOR_ALTA, OUTPUT);
        pinMode(_SECUENCIA, OUTPUT);
        pinMode(_PUERTA, OUTPUT);
        pinMode(_DESAGUE, OUTPUT);
        pinMode(_TAMBOR_DEPOSITO1_FRIA, OUTPUT);
        pinMode(_TAMBOR_DEPOSITO1_CALIENTE, OUTPUT);
        pinMode(_DEPOSITO2, OUTPUT);
        pinMode(_DEPOSITO3, OUTPUT);
        pinMode(_BUTT_TOP, INPUT_PULLUP);
        pinMode(_BUTT_BOT, INPUT_PULLUP);

        digitalWrite(_MOTOR_BAJA, LOW);
        digitalWrite(_MOTOR_ALTA, LOW);
        digitalWrite(_SECUENCIA, LOW);
        digitalWrite(_PUERTA, LOW);
        digitalWrite(_DESAGUE, LOW);
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, LOW);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, LOW);
        digitalWrite(_DEPOSITO2, LOW);
        digitalWrite(_DEPOSITO3, LOW);
    }

    void STOP_M()
    {
        digitalWrite(_MOTOR_BAJA, LOW);
        digitalWrite(_MOTOR_ALTA, LOW);
        digitalWrite(_SECUENCIA, LOW);
    }

    void val_off()
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, LOW);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, LOW);
        digitalWrite(_DEPOSITO2, LOW);
        digitalWrite(_DEPOSITO3, LOW);
    }

    void CENTRIFUGADO()
    {
        drenado();
        digitalWrite(_SECUENCIA, LOW);
        digitalWrite(_MOTOR_BAJA, LOW);
        digitalWrite(_MOTOR_ALTA, HIGH);
    }
    void drenado()
    {
        digitalWrite(_DESAGUE, LOW);
    }

    void no_drenado()
    {
        digitalWrite(_DESAGUE, HIGH);
    }

    /// BANDERA PARA MODIFICAR EL CONTROL DE LA NUEVA PUERTA /////////////////////////////////////////////////
    void r_continental_on_1()
    {
        digitalWrite(_PUERTA_OFF_110, LOW);
    }
    void r_continental_off_1()
    {
        digitalWrite(_PUERTA_OFF_110, HIGH);
    }
    void r_continental_on_2()
    {
        digitalWrite(_PUERTA_ON_110, LOW);
    }
    void r_continental_off_2()
    {
        digitalWrite(_PUERTA_ON_110, HIGH);
    }
    void PUERTA_OFF()
    {
        /////////////////////////////

#if activacion_puerta == 0
        digitalWrite(_CONTROL_VAC, LOW);
        delay(500);
        digitalWrite(_PUERTA_OFF_110, HIGH);
        delay(500);
        digitalWrite(_PUERTA_OFF_110, LOW);
        delay(500);
#endif

        //////////////////////////////
        digitalWrite(_PUERTA, LOW);
    }
    /// ANDERA PARA MODIFICAR EL CONTROL DE LA NUEVA PUERTA /////////////////////////////////////////////////
    void PUERTA_ON()
    {

////////////////////////////
#if activacion_puerta == 0
        delay(500);
        digitalWrite(_CONTROL_VAC, HIGH);
        delay(500);
        digitalWrite(_PUERTA_ON_110, HIGH);
        delay(500);
        digitalWrite(_PUERTA_ON_110, LOW);
#endif

        ////////////////////////////
        digitalWrite(_PUERTA, HIGH);
    }

    void IZQUIERDA_M()
    {
        digitalWrite(_SECUENCIA, HIGH);
        digitalWrite(_MOTOR_BAJA, LOW);
        digitalWrite(_MOTOR_ALTA, LOW);
    }
    void DERECHA_M()
    {
        digitalWrite(_SECUENCIA, LOW);
        digitalWrite(_MOTOR_BAJA, HIGH);
        digitalWrite(_MOTOR_ALTA, LOW);
    }

    void agua_fria_centrifugado() // lavado
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, HIGH);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, LOW);
        digitalWrite(_DEPOSITO2, HIGH);
        digitalWrite(_DEPOSITO3, LOW);
    }

    void agua_fria() // lavado
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, HIGH);
        delay(1000);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, LOW);
        digitalWrite(_DEPOSITO2, HIGH);
        delay(700);
        digitalWrite(_DEPOSITO3, LOW);
        no_drenado();
    }
    void agua_tibia() // lavado
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, HIGH);
        delay(1000);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, HIGH);
        delay(1000);
        digitalWrite(_DEPOSITO2, HIGH);
        delay(700);
        digitalWrite(_DEPOSITO3, LOW);
        no_drenado();
    }
    void agua_caliente() // lavado
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, LOW);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, HIGH);
        delay(1000);
        digitalWrite(_DEPOSITO2, HIGH);
        delay(700);
        digitalWrite(_DEPOSITO3, LOW);
        no_drenado();
    }
    void enjuague_final() // enjuague final
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, HIGH);
        delay(1000);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, LOW);
        digitalWrite(_DEPOSITO2, HIGH);
       delay(1000);
        digitalWrite(_DEPOSITO3, HIGH);
        delay(700);
        no_drenado();
    }

    void agua_fria_enjuagues() // enjuagues
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, HIGH);
        delay(1000);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, LOW);
        digitalWrite(_DEPOSITO2, HIGH);
        delay(700);
        digitalWrite(_DEPOSITO3, LOW);
        no_drenado();
    }
    void agua_tibia_enjuagues() // enjuagues
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, HIGH);
        delay(1000);
        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, HIGH);
        delay(700);
        digitalWrite(_DEPOSITO2, HIGH);
        digitalWrite(_DEPOSITO3, LOW);
        no_drenado();
    }
    void agua_caliente_enjuagues() // enjuagues
    {
        digitalWrite(_TAMBOR_DEPOSITO1_FRIA, LOW);

        digitalWrite(_TAMBOR_DEPOSITO1_CALIENTE, HIGH);
        delay(1000);
        digitalWrite(_DEPOSITO2, HIGH);
        delay(700);
        digitalWrite(_DEPOSITO3, LOW);
        no_drenado();
    }
};

#endif
#endif