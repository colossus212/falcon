#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "cli.h"
#include "pid.h"
#include "clksys_driver.h"
#include "shared.h"

void run_cmd(float val, char * (* function_ptr)(float))
{
    char * status = function_ptr(val);
    printf("%s\n\r", status);
}

char * help(float val)
{
    return "\n\r\n\rAvailable commands:\n\r\
            \r\treboot - reboot the mcu\n\r\
            \r\tprint - display the print commands\n\r\
            \r\tprintpid - print pid structs\n\r\
            \r\tprintmot - print motor packets\n\r\
            \r\tprintimu - print imu packets\n\r\
            \r\tprintbat - print battery voltage\n\r\
            \r\tmot[1-4] <uint16_t> - set motor target value (0-65536)\n\r\
            \r\tled[1-4][r, g]_[on, off] - turn led on or off\n\r\
            \r\tclear - clear the screen\n\r\
            \r\tkp <float> - set kp\n\r\
            \r\tki <float> - set ki\n\r\
            \r\tkd <float> - set kd\n\r\
            \r\thelp - print this message\n\r";
}

char * do_nothing(float val)
{
    return NULL;
}

char * reboot(float val)
{
    printf("rebooting in %f\n\r", val);
    _delay_s((int)val);
    CCPWrite(&RST_CTRL, RST_SWRST_bm);
    return;
}

char * green_led_on(float val)
{
    switch ((int)val)
    {
        case 1:
            LED_1_GREEN_ON();
            return "LED 1 GREEN ON";
        case 2:
            LED_2_GREEN_ON();
            return "LED 2 GREEN ON";
        case 3:
            LED_3_GREEN_ON();
            return "LED 3 GREEN ON";
        case 4:
            LED_4_GREEN_ON();
            return "LED 4 GREEN ON";
        default:
            return "Invalid LED";
    }
}

char * print_pid(struct pid_info * pid)
{
}

char * print(float val)
{
    return "\n\r\n\rAvailable print commands:\n\r\tprintpid\n\r\tprintmot\n\r\tprintimu\n\r\tprintbat\n\r";
}

//char * printmotpkts(volatile struct mot_tx_pkt_t * tx_pkt, volatile struct mot_rx_pkt_t * rx_pkt)
char * printmot(void)
{   
    char * status;
    sprintf(status, "\n\r");
    mot_tx->crc = crc((char *)mot_tx, 9, 7); //calculate the crc on the first 9 bytes of motor packet with divisor 7
    printf(status, "\n\r");
    printf(status, "mot_tx:\t\tmot_rx:\n\r");
    printf(status, "\tstart:   %#02x\t\tstart:   %#02x\n\r", mot_tx->start, mot_rx->start);
    printf(status, "\ttgt_1: %6lu\t\tspd_1: %6lu\n\r", (uint32_t)mot_tx->tgt_1, (uint32_t)mot_rx->spd_1);
    printf(status, "\ttgt_1: %6lu\t\tspd_2: %6lu\n\r", (uint32_t)mot_tx->tgt_2, (uint32_t)mot_rx->spd_2);
    printf(status, "\ttgt_1: %6lu\t\tspd_3: %6lu\n\r", (uint32_t)mot_tx->tgt_3, (uint32_t)mot_rx->spd_3);
    printf(status, "\ttgt_1: %6lu\t\tspd_4: %6lu\n\r", (uint32_t)mot_tx->tgt_4, (uint32_t)mot_rx->spd_4);
    printf(status, "\tcrc:   %6d\t\tcrc:   %6d\n\r", mot_tx->crc, mot_rx->crc);
    return status;
}

void process_rx_buf(volatile char * rx_buf)
{
    char cmd[64];
    float val = 0;
    cmd[0] = '\0';
    sscanf(rx_buf, "%s%f", cmd, &val);
    //printf("\n\rcommand: %s\n\rvalue: %d", cmd, val);
    else if(strcmp(cmd, "printmot") == 0) { print_mot_pkts(&mot_tx, &mot_rx); }
    else if(strcmp(cmd, "printimu") == 0) { print_imu_pkts(&imu_tx, &imu_rx); }
    else if(strcmp(cmd, "printbat") == 0) { printf("\n\rbat_voltage_raw: %d", bat_voltage_raw); printf("\n\rbat_voltage_human: %0.4f", (double)bat_voltage_human); }
    else if(strcmp(cmd, "print_status") == 0) { if(print_status_flag == 0)print_status_flag = 1; else print_status_flag = 0; }
    else if(strcmp(cmd, "start") == 0) { mot_tx.start = (uint8_t)val; }
    else if(strcmp(cmd, "mot1") == 0) { mot_tx.tgt_1 = (uint16_t)val; }
    else if(strcmp(cmd, "mot2") == 0) { mot_tx.tgt_2 = (uint16_t)val; }
    else if(strcmp(cmd, "mot3") == 0) { mot_tx.tgt_3 = (uint16_t)val; }
    else if(strcmp(cmd, "mot4") == 0) { mot_tx.tgt_4 = (uint16_t)val; }
    else if(strcmp(cmd, "motcrc") == 0) { mot_tx.crc = 47; }
    else if(strcmp(cmd, "led1g_on") == 0) { LED_1_GREEN_ON(); }
    else if(strcmp(cmd, "led2g_on") == 0) { LED_2_GREEN_ON(); }
    else if(strcmp(cmd, "led3g_on") == 0) { LED_3_GREEN_ON(); }
    else if(strcmp(cmd, "led4g_on") == 0) { LED_4_GREEN_ON(); }
    else if(strcmp(cmd, "led1r_on") == 0) { LED_1_RED_ON(); }
    else if(strcmp(cmd, "led2r_on") == 0) { LED_2_RED_ON(); }
    else if(strcmp(cmd, "led3r_on") == 0) { LED_3_RED_ON(); }
    else if(strcmp(cmd, "led4r_on") == 0) { LED_4_RED_ON(); }
    else if(strcmp(cmd, "led1g_off") == 0) { LED_1_GREEN_OFF(); }
    else if(strcmp(cmd, "led2g_off") == 0) { LED_2_GREEN_OFF(); }
    else if(strcmp(cmd, "led3g_off") == 0) { LED_3_GREEN_OFF(); }
    else if(strcmp(cmd, "led4g_off") == 0) { LED_4_GREEN_OFF(); }
    else if(strcmp(cmd, "led1r_off") == 0) { LED_1_RED_OFF(); }
    else if(strcmp(cmd, "led2r_off") == 0) { LED_2_RED_OFF(); }
    else if(strcmp(cmd, "led3r_off") == 0) { LED_3_RED_OFF(); }
    else if(strcmp(cmd, "led4r_off") == 0) { LED_4_RED_OFF(); }
    else if(strcmp(cmd, "help") == 0) { printf(help); }
    else if(strcmp(cmd, "clear") == 0) { printf("%c", 12); }
    else if(strcmp(cmd, "kp") == 0) { pid_set_kp(&pid, val); }
    else if(strcmp(cmd, "ki") == 0) { pid_set_ki(&pid, val); }
    else if(strcmp(cmd, "kd") == 0) { pid_set_kd(&pid, val); }
    else if(strcmp(cmd, "target") == 0) { pid_set_target(&pid, val); }
    else if(strcmp(cmd, "print_pid") == 0) { print_pid_info(&pid); }
    else if(strcmp(cmd, "reset_i") == 0) { pid_reset_i(&pid); }
    else if(strcmp(cmd, "request_imu") == 0) { request_imu_pkt(); }
    else if(strcmp(cmd, "init_imu_rx") == 0) { init_imu_rx_pkt(&imu_rx); }
    else if(strcmp(cmd, "write") == 0) { printf("\n\r\n\r%d\n\r", spi_write_read((uint8_t)val, SS1)); }
    else { printf("\n\rcommand not found: %s", cmd); }
}



//void print_imu_pkts(volatile struct imu_tx_pkt_t * tx_pkt, volatile struct imu_rx_pkt_t * rx_pkt)
void printimu(void)
{
    char * status;
    sprintf(status, "%s\n\r\n\r");
    sprintf(status, "%simu_tx_pkt:\t\timu_rx_pkt:\n\r");
    sprintf(status, "%s\tstart:   %#02x\t        start:       %#02x\n\r",     status, tx_pkt->start,      rx_pkt->start);
    sprintf(status, "%s\t                        chksum:    %6d\n\r",     status, rx_pkt->chksum);
    sprintf(status, "%s\t                        pitch_tmp: %6d\n\r",     status, rx_pkt->pitch_tmp);
    sprintf(status, "%s\t                        pitch:     %6d\n\r",     status, rx_pkt->pitch);
    sprintf(status, "%s\t                        yaw:       %6d\n\r",     status, rx_pkt->yaw);
    sprintf(status, "%s\t                        yaw_tmp:   %6d\n\r",     status, rx_pkt->yaw_tmp);
    sprintf(status, "%s\t                        z_accel:   %6d\n\r",     status, rx_pkt->z_accel);
    sprintf(status, "%s\t                        y_accel:   %6d\n\r",     status, rx_pkt->y_accel);
    sprintf(status, "%s\t                        x_accel:   %6d\n\r",     status, rx_pkt->x_accel);
    sprintf(status, "%s\t                        roll:      %6d\n\r",     status, rx_pkt->roll);

    sprintf(status, "%s\n\r%02X %02X %04X %04X %04X %04X %04X %04X %04X %04X\n\r", status, rx_pkt->start, rx_pkt->chksum, rx_pkt->pitch_tmp, rx_pkt->pitch, rx_pkt->yaw, rx_pkt->yaw_tmp, rx_pkt->z_accel, rx_pkt->y_accel, rx_pkt->x_accel, rx_pkt->roll);
    return status;
}

void printbat(void)
{
    char * status;
    sprintf(status, "\n\rbattery: %0.4fV\n\r", (double)bat_voltage_human);
    return status;
}

void printstatus(void)
{
    printf("%c", 12);
    print_pid_info(&pid);
    print_mot_pkts(&mot_tx, &mot_rx);
    print_imu_pkts(&imu_tx, &imu_rx);
    print_bat();
}
