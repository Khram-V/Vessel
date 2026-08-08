#include <stdio.h>
#include <math.h>

/* Константы */
#define G 9.80665

/* Температура насыщения по давлению (P в кПа) */
double saturation_temperature_kpa(double P_kpa) {
    const double A = 16.26205;
    const double B = 3799.887;
    const double C = 273.15;

    if (P_kpa <= 0.0) {
        return -273.15; /* ошибка */
    }
    return (B / (A - log(P_kpa))) - C;
}

/* Плотность воды при температуре T (°C) в кг/м^3 */
double water_density_kgm3(double T_c) {
    if (T_c < 0.0 || T_c > 150.0) {
        /* вне диапазона аппроксимации */
        return 998.0;
    }
    return 999.85 + 0.042 * T_c - 0.00304 * T_c * T_c;
}

/* Скорость потока по расходу и диаметру */
double flow_velocity_ms(double Q_m3s, double D_m) {
    if (D_m <= 0.0) return 0.0;
    double A = M_PI * D_m * D_m / 4.0;
    return Q_m3s / A;
}

/* Потери на трение (Дарси-Вейсбах), f — коэффициент трения */
double pressure_loss_friction_pa(double f, double L_m, double D_m,
                                  double rho_kgm3, double v_ms) {
    return f * (L_m / D_m) * 0.5 * rho_kgm3 * v_ms * v_ms;
}

/* Местные потери (через суммарный коэффициент xi) */
double pressure_loss_local_pa(double xi, double rho_kgm3, double v_ms) {
    return xi * 0.5 * rho_kgm3 * v_ms * v_ms;
}
#include <Windows.h> //Con.h>

int main() {
    SetConsoleCP( CP_UTF8 );        // 1251
    SetConsoleOutputCP( CP_UTF8 );  // 1251
    /* Входные параметры */
    double P_res_kpa = 80.0;      /* давление в резервуаре (пониженное) */
    double h_m = 3.0;             /* перепад высот: градирня выше нагревателя */
    double Q_m3s = 0.001;         /* расход конденсата: 1 л/с = 0.001 м^3/с */
//18:46
    double D_m = 0.05;            // диаметр трубы: 50 мм /
    double L_m = 10.0;            // длина трубы возврата /
    double f = 0.02;              // коэффициент трения (оценка) /
    double xi = 8.0;              // суммарный коэффициент местных сопротивлений */

/* 1. Температура кипения в резервуаре */
double T_sat = saturation_temperature_kpa(P_res_kpa);
printf("Температура насыщения при %.1f кПа: %.1f °C\n", P_res_kpa, T_sat);

/* 2. Плотность конденсата при этой температуре */
double rho = water_density_kgm3(T_sat);
printf("Плотность конденсата при %.1f °C: %.1f кг/м^3\n", T_sat, rho);

/* 3. Скорость потока */
double v = flow_velocity_ms(Q_m3s, D_m);
printf("Скорость потока: %.3f м/с\n", v);

/* 4. Гравитационный напор */
double dP_grav = rho * G * h_m;
printf("Гравитационный напор: %.0f Па (%.2f кПа)\n", dP_grav, dP_grav / 1000.0);

/* 5. Потери на трение и местные сопротивления */
double dP_fric = pressure_loss_friction_pa(f, L_m, D_m, rho, v);
double dP_local = pressure_loss_local_pa(xi, rho, v);
double dP_total = dP_fric + dP_local;
printf("Потери на трение: %.0f Па\n", dP_fric);
printf("Местные потери: %.0f Па\n", dP_local);
printf("Суммарные потери: %.0f Па (%.2f кПа)\n", dP_total, dP_total / 1000.0);

/* 6. Проверка условия циркуляции */
  if (dP_grav > dP_total) {
    printf("\nЦиркуляция возможна: гравитационный напор достаточен.\n");
    printf("Запас: %.0f Па\n", (int)(dP_grav - dP_total));
  } else {
    printf("\nЦиркуляция НЕВОЗМОЖНА: напор недостаточен.\n");
    printf("Не хватает: %.0f Па\n", (int)(dP_total - dP_grav));
  } return 0;
}
