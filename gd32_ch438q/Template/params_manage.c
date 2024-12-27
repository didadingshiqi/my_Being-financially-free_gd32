#include <stdio.h>
#include <string.h>
#include "params_manage.h"
 
#define LAST_PAGE_ADDR  0x0807F800
static uint16_t store_Data[1024];

 uint32_t MyFLASH_Readword (uint32_t Address)
{
    return *( ( uint32_t *)(Address) );
}
 
//ע�⣺��ַΪС�˴洢
uint16_t MyFLASH_ReadHalfword (uint32_t Address)
{
    return *(( uint16_t *)(Address) );
}
 
uint8_t MyFLASH_ReadByte (uint32_t Address)
{
    return * ( ( uint8_t * ) (Address)) ;
}
 
void MyFLASH_EraseAllPages (void)
{
    fmc_unlock () ;
    fmc_mass_erase ( ) ;
    fmc_lock () ;
}
 
void MyFLASH_ErasePage (uint32_t PageAddress)
{
    fmc_unlock ( );
    fmc_page_erase (PageAddress ) ;
    fmc_lock ();
}
 
void MyFLASH_Programword (uint32_t Address, uint32_t Data)
{
    fmc_unlock ( ) ;
    fmc_word_program (Address,Data) ;
    fmc_lock ( ) ;
}
 
void MyFIASH_ProgramHalfword (uint32_t Address,uint16_t Data)
{
    fmc_unlock ( ) ;
    fmc_halfword_program (Address,Data) ;
    fmc_lock ();
}
 

//�ϵ���ó�ʼ��������Ȼ��Ϳ��Լ���֮ǰ�洢��flash�е�����
void store_Init (void)
{
	uint16_t i;
	
    //0x0800Fc00���һҳ���׵�ַ��OxA5A5Ϊ���ж�֮ǰ�Ƿ�洢����
    if (MyFLASH_ReadHalfword(LAST_PAGE_ADDR)!= 0xA5A5)
    {
        MyFLASH_ErasePage (LAST_PAGE_ADDR);
        MyFIASH_ProgramHalfword(LAST_PAGE_ADDR,0xA5A5);
        // һҳ�Ĵ�С��2K,�洢���������С�ǰ��֡���һλ�洢��־λ����������λ��0
        for (i = 1; i <1024; i ++)
        {
            MyFIASH_ProgramHalfword (LAST_PAGE_ADDR + i * 2,0x0000);
        }
    }
    for (i = 0; i <1024; i ++)
    {
        store_Data [i] = MyFLASH_ReadHalfword (LAST_PAGE_ADDR + i * 2);
    }
}
 
//���ñ��溯���ͻὫ����д��flash�洢����
void store_save (void)
{
	uint16_t i;
    MyFLASH_ErasePage (LAST_PAGE_ADDR) ;
    for (i = 0; i < 1024; i ++)
    {
        MyFIASH_ProgramHalfword (LAST_PAGE_ADDR + i * 2,store_Data[i]);
    }
}
 
void store_clear (void)
{
	uint16_t i;
    for (i = 1; i < 1024; i ++)
    {
        store_Data[i] = 0x0000;
    }
    store_save () ;
}
