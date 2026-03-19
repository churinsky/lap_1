#include <Arduino.h>
class Button
#define buzzer 13
{
private:
    int _button;
    int _menu;

public:
    Button(int button)
    {
        _button = button;
        pinMode(_button, INPUT_PULLUP);
    }
    bool isPressed()
    {
        if (!digitalRead(_button))
        {
            ton();
        }

        return (!digitalRead(_button));
    }
    int pulses(int pulse)
    {
        delay(150); // rebotes al precionar el boton
        _menu = pulse;

        if (_menu < 2)
        {
            return 1;
        }
        else
        {
            if (_menu > 4)
            {
                return 1;
            }
            else
            {
                return pulse;
            }
        }
    }

private:
    void ton()
    {
        tone(buzzer, 2000);
        delay(100);
        noTone(buzzer);
    }
};
