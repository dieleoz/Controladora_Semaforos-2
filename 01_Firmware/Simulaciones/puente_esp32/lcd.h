// ===== Validacion_Automatico/lcd.h =====
// Sustituto de lcd.h para compilar modo_automatico.cpp en el PC.
//
// modo_automatico.cpp SOLO llama a estas dos funciones de toda la pantalla. Un
// sustituto que ofreciera las demas invitaria a que este arnes creciera hacia
// emular la LCD, que ya tiene su propio arnes (Validacion_LCD) sobre el lcd.cpp
// real. Aqui lo que importa es el CICLO, no el dibujo; las definiciones (en
// arnes_automatico.cpp) solo registran la ultima llamada, para poder comprobar que
// la pantalla se refresca cuando el estado cambia.
#pragma once

void lcd_dibujarAutomatico(const char* nombreEstado, int minRojo, int minVerde);
void lcd_dibujarConfigValor(const char* etiqueta, int valor, const char* unidad);
