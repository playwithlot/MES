//CRC16 ���㺯��
#include "CRC16.h"

#define CRC_CODE 0x1021 //crc����ʽ
//crc16У��
uint16_t CRC16_Verify(uint16_t InitValue, uint8_t *ptr, uint32_t len)
{
    uint16_t crc = InitValue; //��ʼ��crcֵ
    uint8_t i;

    while (len--)
    {
        for (i = 0x80; i != 0; i >>= 1)
        {
            if ((crc & 0x8000) != 0)
            {
                crc <<= 1;
                crc ^= CRC_CODE;
            }
            else
            {
                crc <<= 1;
            }

            if ((*ptr & i) != 0)
            {
                crc ^= CRC_CODE;
            }
        }

        ptr++;
    }

    return (crc);
}

uint8_t CheckXor(uint8_t *p, uint16_t length)
{
    //���У��,�ɹ�����0�����󷵻�����ֵ
    uint8_t temp;
    uint16_t i;

    temp = 0;
    for (i = 0; i < length; i++)
    {
        temp ^= *p;
        p++;
    }
    return (temp);
}
//�������У��
uint8_t CalcXor(uint8_t *p, uint16_t length, uint8_t xorinit)
{
    uint8_t temp;
    uint16_t i;

    temp = xorinit;
    for (i = 0; i < length; i++)
    {
        temp ^= *p;
        p++;
    }
    return (temp);
}

uint8_t crc8(uint8_t *ptr, uint32_t len)
{
    uint8_t i; 
    uint8_t crc=0x00;/* ����ĳ�ʼcrcֵ */ 
 
    while(len--)
    {
        crc ^= *ptr++;  /* ÿ��������Ҫ������������,������ָ����һ���� */  
        for (i=8; i>0; --i)   /* ������μ�����������һ���ֽ�crcһ�� */  
        { 
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc = (crc << 1);
        }
    }
 
    return (crc); 
}

//modebus ��׼crc
uint16_t CRC16_modbus(uint8_t *ptr, uint32_t len)
{
	uint16_t crc = 0xFFFF;
	uint32_t pos;
	uint8_t i;
    for (pos = 0; pos < len; pos++)
    {
        crc ^= ptr[pos]; // XOR byte into least sig. byte of crc
        for (i = 8; i != 0; i--)   // Loop over each bit
        {
            if ((crc & 0x0001) != 0)   // If the LSB is set
            {
                crc >>= 1; // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else // Else LSB is not set
            {
                crc >>= 1;    // Just shift right
            }
        }
    }
 
    //�ߵ��ֽ�ת��
    //crc = ((crc & 0x00ff) << 8) | ((crc & 0xff00) >> 8);
    return crc;
}
/******************************************************************************
**  						  �ĵ�����
******************************************************************************/
