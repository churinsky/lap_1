#ifndef _SELECCION_PARAMETROS_H_
#define _SELECCION_PARAMETROS_H_

#include <Arduino.h>
#include "EEPROM.h"
// parametros
int temperatura_1;

int LLENADO_AGIpre_1;
int DESAGUE_AGIpre_1;
int CENTRIFUpre_1;
int nivelde_llenado_prelavado_1;
//--------------------------------------
int LLENADO_AGIlav_1;
int DESAGUE_AGIlav_1;
int CENTRIFUlav_1;
int nivelde_llenado_lavado_1;
//--------------------------------------
int LLENADO_AGIeng_1;
int DESAGUE_AGIeng_1;
int CENTRIFUeng_1;
int nivelde_llenado_enjugaue_1;
//--------------------------------------
int LLENADO_AGIex1_1;
int DESAGUE_AGIex1_1;
int CENTRIFUex1_1;
int nivel_de_llenado_ennjuague_extra_1_1;
//-------------------------------------------
int LLENADO_AGIex2_1;
int DESAGUE_AGIex2_1;
int CENTRIFUex2_1;
int nivel_de_llenado_ennjuague_extra_2_1;
//--------------------------------------------
int LLENADO_AGIfin_1;
int DESAGUE_AGIfin_1;
int CENTRIFUfin_1;
int nivel_de_llenado_ennjuague_final_1;
//-----------------------------------------
int TIEMPO_CENTRIFUGADO_FINAL_1;
int nivel_de_llenado_centrifugado_1;
// ------------------------------------------
int tiempo_giro_izquierda_1;
int tiempo_giro_derecha_1;
int tiempo_reposo_1;
void paramreos_eeprom()
{
    EEPROM.update(33, 1); 
    EEPROM.update(101, 1);   // nivel de agua_ normal 165 mediana chica 163
    EEPROM.update(102, 1); // nnivel de agua_bajo 166 mediana chica 164
    EEPROM.update(103, 163); // nivel de agua_alto 166 mediana chica 164
                             ////////////////////////////////////////////ciclo 1
    EEPROM.update(400, 0);   // 
    EEPROM.update(401, 0);   // 0
    EEPROM.update(402, 0);   //

    EEPROM.update(104, 13); // 8
    EEPROM.update(105, 1);  // 1
    EEPROM.update(106, 0);  //2

    EEPROM.update(107, 0); // 0
    EEPROM.update(108, 0); // 0
    EEPROM.update(109, 0);

    EEPROM.update(110, 0); //0
    EEPROM.update(111, 0);
    EEPROM.update(112, 0);

    EEPROM.update(113, 0);//0
    EEPROM.update(114, 0);
    EEPROM.update(115, 0);

    EEPROM.update(116, 4); // 4
    EEPROM.update(117, 1); // 1
    EEPROM.update(118, 0);//2

    EEPROM.update(119, 8);
    ////////////////////////////////////
    EEPROM.update(120, 15);
    EEPROM.update(121, 15);
    EEPROM.update(122, 6);
    ///////////////////////////////////////////////////////ciclo2

    EEPROM.update(403, 0); // 8
    EEPROM.update(404, 0); // 1
    EEPROM.update(405, 0); //

    EEPROM.update(123, 13);
    EEPROM.update(124, 1);
    EEPROM.update(125, 0);

    EEPROM.update(126, 0);
    EEPROM.update(127, 0);
    EEPROM.update(128, 0);

    EEPROM.update(129, 0);
    EEPROM.update(130, 0);
    EEPROM.update(131, 0);

    EEPROM.update(132, 0);
    EEPROM.update(133, 0);
    EEPROM.update(134, 0);

    EEPROM.update(135, 4);
    EEPROM.update(136, 1);
    EEPROM.update(137, 0);

    EEPROM.update(138, 8);
    //////////////////////////////////////////////////////////////////////
    EEPROM.update(139, 6);
    EEPROM.update(140, 6);
    EEPROM.update(141, 10);
    /////////////////////////////////////////////////////ciclo3

    EEPROM.update(406, 0); // 8
    EEPROM.update(407, 0); // 1
    EEPROM.update(408, 0); //

    EEPROM.update(142, 4);
    EEPROM.update(143, 1);
    EEPROM.update(144, 0);

    EEPROM.update(145, 8);
    EEPROM.update(146, 1);
    EEPROM.update(147, 0);

    EEPROM.update(148, 0);
    EEPROM.update(149, 0);
    EEPROM.update(150, 0);

    EEPROM.update(151, 0);
    EEPROM.update(152, 0);
    EEPROM.update(153, 0);

    EEPROM.update(154, 4);
    EEPROM.update(155, 1);
    EEPROM.update(156, 0);

    EEPROM.update(157, 8);
    //////////////////////////////////////////////////////////////////////////////
    EEPROM.update(158, 15);
    EEPROM.update(159, 15);
    EEPROM.update(160, 3);

    EEPROM.update(161, 40); // error llenado tiempo

    EEPROM.update(162, 3);  // ciclo_1
    EEPROM.update(163, 3);  // ciclo_2
    EEPROM.update(164, 3);  // ciclo_3
    EEPROM.update(190, 1);  // ciclo_4
    EEPROM.update(165, 0);  // precio tibia
    EEPROM.update(166, 0);  // precio caliente
    EEPROM.update(167, 0);  // nivel2_agua
    EEPROM.update(168, 0);  // nivel3_agua
    EEPROM.update(169, 5); // valor de la moneda

    ///////////////////////usos
    EEPROM.update(195, 0);   // debug uso
    EEPROM.update(196, 21);  // bloqueo de caducidad
    EEPROM.update(191, 55);  // clav4
    EEPROM.update(192, 36);  // clave
    EEPROM.update(193, 98);  // clave
    EEPROM.update(194, 211); // clave
    EEPROM.update(1002, 0);  // uso mes1
    EEPROM.update(1003, 0);  // uso mes2
    EEPROM.update(1004, 0);  // uso mes3
    EEPROM.update(1005, 0);  // uso mes4
    EEPROM.update(1006, 0);  // uso mes 5
    EEPROM.update(1007, 0);  // uso mes 6
    EEPROM.update(1008, 0);  // uso mes 6
    EEPROM.update(1009, 0);  // usos
    EEPROM.update(1010, 0);  // usos
                             ////////////////////////////////////////////////////////////////////c4
    EEPROM.update(409, 0);   // 8
    EEPROM.update(410, 0);   // 1
    EEPROM.update(411, 0);   //

    EEPROM.update(170, 0);
    EEPROM.update(171, 0);
    EEPROM.update(172, 0);

    EEPROM.update(173, 0);
    EEPROM.update(174, 0);
    EEPROM.update(175, 0);

    EEPROM.update(176, 0);
    EEPROM.update(177, 0);
    EEPROM.update(178, 0);

    EEPROM.update(179, 0);
    EEPROM.update(180, 0);
    EEPROM.update(181, 0);

    EEPROM.update(182, 0);
    EEPROM.update(183, 0);
    EEPROM.update(184, 0);

    EEPROM.update(185, 9);
    //////////////////////////////////////////////////////////////////////////////
    EEPROM.update(186, 10);
    EEPROM.update(187, 10);
    EEPROM.update(188, 3);
    ///////////////////////////////////////////////////////////////////////////////////////////////
    EEPROM.update(200, 60); // litros de agua etapa 1
    EEPROM.update(201, 60); // litros de agua etapa 2
    EEPROM.update(202, 60); // litros de agua etapa 3
    EEPROM.update(203, 60); // litros de agua etapa 4
    EEPROM.update(204, 60); // litros de agua etapa 5
    EEPROM.update(205, 60); // litros de agua etapa 6
    EEPROM.update(206, 0); // litros de agua etapa 7
    ////////////////////////////////////////////////////////
    // EEPROM.update(1001, 1); // opl por mientras
}
void seleccion_agua(int parametros) // nivel del agua
{
    if (parametros == 1)

    {
        nivelde_llenado_prelavado_1 = EEPROM.read(101);
        nivelde_llenado_lavado_1 = EEPROM.read(101);
        nivelde_llenado_enjugaue_1 = EEPROM.read(101);
        nivel_de_llenado_ennjuague_extra_1_1 = EEPROM.read(101);
        nivel_de_llenado_ennjuague_extra_2_1 = EEPROM.read(101);
        nivel_de_llenado_ennjuague_final_1 = EEPROM.read(101);
        nivel_de_llenado_centrifugado_1 = 0;
        //Serial.println("aguna_N1");
    }
    if (parametros == 2)
    {
        nivelde_llenado_prelavado_1 = EEPROM.read(102);
        nivelde_llenado_lavado_1 = EEPROM.read(102);
        nivelde_llenado_enjugaue_1 = EEPROM.read(102);
        nivel_de_llenado_ennjuague_extra_1_1 = EEPROM.read(102);
        nivel_de_llenado_ennjuague_extra_2_1 = EEPROM.read(102);
        nivel_de_llenado_ennjuague_final_1 = EEPROM.read(102);
        nivel_de_llenado_centrifugado_1 = 0;
       // Serial.println("aguna_N2");
    }
    if (parametros == 3)
    {
        nivelde_llenado_prelavado_1 = EEPROM.read(103);
        nivelde_llenado_lavado_1 = EEPROM.read(103);
        nivelde_llenado_enjugaue_1 = EEPROM.read(103);
        nivel_de_llenado_ennjuague_extra_1_1 = EEPROM.read(103);
        nivel_de_llenado_ennjuague_extra_2_1 = EEPROM.read(103);
        nivel_de_llenado_ennjuague_final_1 = EEPROM.read(103);
        nivel_de_llenado_centrifugado_1 = 0;
       // Serial.println("aguna_N3");
    }
}

void seleccion_temperatura(int parametros) // temperatura del agua
{
    if (parametros == 1)
    {
        temperatura_1 = 0;
     //   Serial.println("temperatura_N1");
    }
    if (parametros == 2)
    {
        temperatura_1 = 1;
     //   Serial.println("temperatura_N2");
    }
    if (parametros == 3)
    {
        temperatura_1 = 2;
     //   Serial.println("temperatura_N3");
    }
}
void seleccion_ciclo(int parametros) // tiempo y duracion de ciclos
{
    if (parametros == 1)
    {
        LLENADO_AGIpre_1 = EEPROM.read(400);
        DESAGUE_AGIpre_1 = EEPROM.read(401);
        CENTRIFUpre_1 = EEPROM.read(402);

        LLENADO_AGIlav_1 = EEPROM.read(104);
        DESAGUE_AGIlav_1 = EEPROM.read(105);
        CENTRIFUlav_1 = EEPROM.read(106);

        LLENADO_AGIeng_1 = EEPROM.read(107);
        DESAGUE_AGIeng_1 = EEPROM.read(108);
        CENTRIFUeng_1 = EEPROM.read(109);

        LLENADO_AGIex1_1 = EEPROM.read(110);
        DESAGUE_AGIex1_1 = EEPROM.read(111);
        CENTRIFUex1_1 = EEPROM.read(112);

        LLENADO_AGIex2_1 = EEPROM.read(113);
        DESAGUE_AGIex2_1 = EEPROM.read(114);
        CENTRIFUex2_1 = EEPROM.read(115);

        LLENADO_AGIfin_1 = EEPROM.read(116);
        DESAGUE_AGIfin_1 = EEPROM.read(117);
        CENTRIFUfin_1 = EEPROM.read(118);

        tiempo_giro_izquierda_1 = EEPROM.read(120);
        tiempo_giro_derecha_1 = EEPROM.read(121);
        tiempo_reposo_1 = EEPROM.read(122);

        TIEMPO_CENTRIFUGADO_FINAL_1 = EEPROM.read(119);

     //   Serial.println("ciclo_N1");
    }
    if (parametros == 2)
    {

        LLENADO_AGIpre_1 = EEPROM.read(403);
        DESAGUE_AGIpre_1 = EEPROM.read(404);
        CENTRIFUpre_1 = EEPROM.read(405);

        LLENADO_AGIlav_1 = EEPROM.read(123);
        DESAGUE_AGIlav_1 = EEPROM.read(124);
        CENTRIFUlav_1 = EEPROM.read(125);

        LLENADO_AGIeng_1 = EEPROM.read(126);
        DESAGUE_AGIeng_1 = EEPROM.read(127);
        CENTRIFUeng_1 = EEPROM.read(128);

        LLENADO_AGIex1_1 = EEPROM.read(129);
        DESAGUE_AGIex1_1 = EEPROM.read(130);
        CENTRIFUex1_1 = EEPROM.read(131);

        LLENADO_AGIex2_1 = EEPROM.read(132);
        DESAGUE_AGIex2_1 = EEPROM.read(133);
        CENTRIFUex2_1 = EEPROM.read(134);

        LLENADO_AGIfin_1 = EEPROM.read(135);
        DESAGUE_AGIfin_1 = EEPROM.read(136);
        CENTRIFUfin_1 = EEPROM.read(137);

        tiempo_giro_izquierda_1 = EEPROM.read(139);
        tiempo_giro_derecha_1 = EEPROM.read(140);
        tiempo_reposo_1 = EEPROM.read(141);

        TIEMPO_CENTRIFUGADO_FINAL_1 = EEPROM.read(138);
     //   Serial.println("ciclo_N2");
    }
    if (parametros == 3)
    {

        LLENADO_AGIpre_1 = EEPROM.read(406);
        DESAGUE_AGIpre_1 = EEPROM.read(407);
        CENTRIFUpre_1 = EEPROM.read(408);

        LLENADO_AGIlav_1 = EEPROM.read(142);
        DESAGUE_AGIlav_1 = EEPROM.read(143);
        CENTRIFUlav_1 = EEPROM.read(144);

        LLENADO_AGIeng_1 = EEPROM.read(145);
        DESAGUE_AGIeng_1 = EEPROM.read(146);
        CENTRIFUeng_1 = EEPROM.read(147);

        LLENADO_AGIex1_1 = EEPROM.read(148);
        DESAGUE_AGIex1_1 = EEPROM.read(149);
        CENTRIFUex1_1 = EEPROM.read(150);

        LLENADO_AGIex2_1 = EEPROM.read(151);
        DESAGUE_AGIex2_1 = EEPROM.read(152);
        CENTRIFUex2_1 = EEPROM.read(153);

        LLENADO_AGIfin_1 = EEPROM.read(154);
        DESAGUE_AGIfin_1 = EEPROM.read(155);
        CENTRIFUfin_1 = EEPROM.read(156);

        tiempo_giro_izquierda_1 = EEPROM.read(158);
        tiempo_giro_derecha_1 = EEPROM.read(159);
        tiempo_reposo_1 = EEPROM.read(160);

        TIEMPO_CENTRIFUGADO_FINAL_1 = EEPROM.read(157);
     //   Serial.println("ciclo_N3");
    }
    if (parametros == 4)
    {

        LLENADO_AGIpre_1 = EEPROM.read(409);
        DESAGUE_AGIpre_1 = EEPROM.read(410);
        CENTRIFUpre_1 = EEPROM.read(411);

        LLENADO_AGIlav_1 = EEPROM.read(170);
        DESAGUE_AGIlav_1 = EEPROM.read(171);
        CENTRIFUlav_1 = EEPROM.read(172);

        LLENADO_AGIeng_1 = EEPROM.read(173);
        DESAGUE_AGIeng_1 = EEPROM.read(174);
        CENTRIFUeng_1 = EEPROM.read(175);

        LLENADO_AGIex1_1 = EEPROM.read(176);
        DESAGUE_AGIex1_1 = EEPROM.read(177);
        CENTRIFUex1_1 = EEPROM.read(178);

        LLENADO_AGIex2_1 = EEPROM.read(179);
        DESAGUE_AGIex2_1 = EEPROM.read(180);
        CENTRIFUex2_1 = EEPROM.read(181);

        LLENADO_AGIfin_1 = EEPROM.read(182);
        DESAGUE_AGIfin_1 = EEPROM.read(183);
        CENTRIFUfin_1 = EEPROM.read(184);

        tiempo_giro_izquierda_1 = EEPROM.read(186);
        tiempo_giro_derecha_1 = EEPROM.read(187);
        tiempo_reposo_1 = EEPROM.read(188);

        TIEMPO_CENTRIFUGADO_FINAL_1 = EEPROM.read(185);
     //   Serial.println("ciclo_N4");
    }
}
#endif