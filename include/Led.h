#include <Arduino.h>

class Led
{
    int _tipo_ciclo_LED;
    int _tipo_temperatura_LED;
    int _nivel_agua_LED;
    int _Inicio_LED;

public:
    Led(int tipo_ciclo_LED, int tipo_temperatura_LED, int nivel_agua_LED, int Inicio_LED)
    {
        _tipo_ciclo_LED = tipo_ciclo_LED;
        pinMode(_tipo_ciclo_LED, OUTPUT);

        _tipo_temperatura_LED = tipo_temperatura_LED;
        pinMode(_tipo_temperatura_LED, OUTPUT);

        _nivel_agua_LED = nivel_agua_LED;
        pinMode(_nivel_agua_LED, OUTPUT);

        _Inicio_LED = Inicio_LED;
        pinMode(_Inicio_LED, OUTPUT);

        digitalWrite(_tipo_ciclo_LED, LOW);
        digitalWrite(_tipo_temperatura_LED, LOW);
        digitalWrite(_nivel_agua_LED, LOW);
        digitalWrite(_Inicio_LED, LOW);
    }
    void ciclo_LED()
    {
        delay(50);
        digitalWrite(_tipo_ciclo_LED, HIGH);
        delay(100);
        digitalWrite(_tipo_ciclo_LED, LOW);
    }
    void temperatura_LED()
    {

        delay(50);
        digitalWrite(_tipo_temperatura_LED, HIGH);
        delay(100);
        digitalWrite(_tipo_temperatura_LED, LOW);
    }
    void agua_LED()
    {
        delay(50);
        digitalWrite(_nivel_agua_LED, HIGH);
        delay(100);
        digitalWrite(_nivel_agua_LED, LOW);
    }
    void inicio_LED()
    {
        delay(50);
        digitalWrite(_Inicio_LED, HIGH);
        delay(100);
        digitalWrite(_Inicio_LED, LOW);
    }
    void prog_LED_on()
    {
        digitalWrite(_Inicio_LED, LOW);
    }
    void prog_LED_off()
    {
        digitalWrite(_Inicio_LED, HIGH);
    }
};
