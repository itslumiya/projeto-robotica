/*
 * File:          Controlador.c
 * Date:
 * Description:
 * Author:
 * Modifications:
 */

/*
 * You may need to add include files like <webots/distance_sensor.h> or
 * <webots/motor.h>, etc.
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/distance_sensor.h>
#include <webots/led.h>
#include <webots/supervisor.h>

/*
 * You may want to add macros here.
 */
#define TIME_STEP 256
#define QtddSensoresProx 8
#define QtddLeds 10
#define QtddCaixa 20

/*
 * This is the main program.
 * The arguments of the main function can be specified by the
 * "controllerArgs" field of the Robot node
 */
int main(int argc, char **argv)
{
  double LeituraSensorProx[QtddSensoresProx];
  double AceleradorDireito = 1.0, AceleradorEsquerdo = 1.0;
  bool EncontrouCaixaLeve = false;
  WbNodeRef caixa[QtddCaixa];
  char nomeCaixa[20] = {0};

  // Inicializa Webots
  wb_robot_init();

  // Configuração dos motores
  WbDeviceTag MotorEsquerdo = wb_robot_get_device("left wheel motor");
  WbDeviceTag MotorDireito = wb_robot_get_device("right wheel motor");

  wb_motor_set_position(MotorEsquerdo, INFINITY);
  wb_motor_set_position(MotorDireito, INFINITY);
  wb_motor_set_velocity(MotorEsquerdo, 0);
  wb_motor_set_velocity(MotorDireito, 0);

  // Configuração dos sensores de proximidade
  WbDeviceTag SensorProx[QtddSensoresProx];
  SensorProx[0] = wb_robot_get_device("ps0");
  SensorProx[1] = wb_robot_get_device("ps1");
  SensorProx[2] = wb_robot_get_device("ps2");
  SensorProx[3] = wb_robot_get_device("ps3");
  SensorProx[4] = wb_robot_get_device("ps4");
  SensorProx[5] = wb_robot_get_device("ps5");
  SensorProx[6] = wb_robot_get_device("ps6");
  SensorProx[7] = wb_robot_get_device("ps7");

  for (int c = 0; c < QtddCaixa; c++)
  {
    sprintf(nomeCaixa, "CAIXA%d", c + 1);
    caixa[c] = wb_supervisor_node_get_from_def(nomeCaixa);
    if (caixa[c] != NULL)
      printf("%d. %s  -  %p\n", c + 1, nomeCaixa, (void *)caixa[c]);
    else
      printf("Falha ao carregar a posição da %s\n", nomeCaixa);
  }
  printf("\n\n CAIXAS OK  \n\n");

  for (int i = 0; i < QtddSensoresProx; i++)
  {
    wb_distance_sensor_enable(SensorProx[i], TIME_STEP);
  }

  // Configuração dos LEDs
  WbDeviceTag Leds[QtddLeds];
  Leds[0] = wb_robot_get_device("led0");
  wb_led_set(Leds[0], -1);

  // Loop principal
  while (wb_robot_step(TIME_STEP) != -1)
  {
    double PosicaoCaixaAntes[QtddCaixa][3];

    // Leitura dos sensores
    for (int i = 0; i < QtddSensoresProx; i++)
    {
      LeituraSensorProx[i] = wb_distance_sensor_get_value(SensorProx[i]) - 60;
    }

    // Leitura da posição das caixas
    for (int c = 0; c < QtddCaixa; c++)
    {
      const double *pos = wb_supervisor_node_get_position(caixa[c]);
      PosicaoCaixaAntes[c][0] = pos[0];
      PosicaoCaixaAntes[c][1] = pos[1];
      PosicaoCaixaAntes[c][2] = pos[2];
    }

    // Alterna estado do LED
    wb_led_set(Leds[0], wb_led_get(Leds[0]) * -1);

    // Lógica de controle do movimento
    if (EncontrouCaixaLeve)
    {
      AceleradorDireito = 1;
      AceleradorEsquerdo = -1;
    }
    else if ((LeituraSensorProx[7] > 1000 || LeituraSensorProx[0] > 1000 || LeituraSensorProx[1] > 1000) && (LeituraSensorProx[2] > 1000 || LeituraSensorProx[3] > 1000))
    {
      AceleradorDireito = 1;
      AceleradorEsquerdo = -1;
    }
    else if ((LeituraSensorProx[7] > 1000 || LeituraSensorProx[0] > 1000 || LeituraSensorProx[6] > 1000) && (LeituraSensorProx[4] > 1000 || LeituraSensorProx[5] > 1000))
    {
      AceleradorDireito = -1;
      AceleradorEsquerdo = 1;
    }
    else if (LeituraSensorProx[7] > 1000 || LeituraSensorProx[0] > 1000)
    {
      AceleradorDireito = -1;
      AceleradorEsquerdo = 1;
    }
    else
    {
      AceleradorDireito = 1;
      AceleradorEsquerdo = 1;
    }

    wb_motor_set_velocity(MotorEsquerdo, 6.28 * AceleradorEsquerdo);
    wb_motor_set_velocity(MotorDireito, 6.28 * AceleradorDireito);

    wb_robot_step(TIME_STEP);

    for (int c = 0; c < QtddCaixa; c++)
    {
      const double *PosicaoCaixaDepois = wb_supervisor_node_get_position(caixa[c]);

      if (fabs(PosicaoCaixaAntes[c][0] - PosicaoCaixaDepois[0]) > 0.001 ||
          fabs(PosicaoCaixaAntes[c][1] - PosicaoCaixaDepois[1]) > 0.001 ||
          fabs(PosicaoCaixaAntes[c][2] - PosicaoCaixaDepois[2]) > 0.001)
      {
        EncontrouCaixaLeve = true;
        printf("\nEncontrou caixa leve!\n");
        printf("CAIXA%d - ANTES: X: %.2f  Y: %.2f  Z: %.2f\n", c + 1, PosicaoCaixaAntes[c][0], PosicaoCaixaAntes[c][1], PosicaoCaixaAntes[c][2]);
        printf("CAIXA%d - DEPOIS: X: %.2f  Y: %.2f  Z: %.2f\n", c + 1, PosicaoCaixaDepois[0], PosicaoCaixaDepois[1], PosicaoCaixaDepois[2]);
        break;
      }
    }
  }

  // Finalização
  wb_robot_cleanup();
  return 0;
}