#ifndef _MENU_H_
#define _MENU_H_

#include <Arduino.h>
#include <TM1637Display.h>
#define CLK 4
#define DIO 2
TM1637Display display(CLK, DIO);
class Menu
{
    int _menu;
    int _displa;

public:
    void cases(int menu, int displa)
    {
        _displa = displa;
        _menu = menu;
        switch (_menu)
        {
        case 0:
            if (_displa == 1)
            {
                display.clear();

                
                display.setBrightness(0x0f);
                display.setSegments(SEG_c1);
            }
            if (_displa == 2)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_t1);
            }
            if (_displa == 3)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_n1);
            }
            break;
        case 1:
            if (_displa == 1)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_c1);
            }
            if (_displa == 2)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_t1);
            }
            if (_displa == 3)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_n1);
            }
            break;
        case 2:
            if (_displa == 1)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_c2);
            }
            if (_displa == 2)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_t2);
            }
            if (_displa == 3)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_n2);
            }
            break;
        case 3:
            if (_displa == 1)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_c3);
            }
            if (_displa == 2)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_t3);
            }
            if (_displa == 3)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_n3);
            }
            break;
        case 4:
            if (_displa == 1)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_c4);
            }

            break;
        default:
            if (_displa == 1)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_c1);
            }
            if (_displa == 2)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_t1);
            }
            if (_displa == 3)
            {
                display.clear();
                display.setBrightness(0x0f);
                display.setSegments(SEG_n1);
            }
            break;
        }
    }
};
#endif