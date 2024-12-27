#include <stdio.h>
#include <string.h>
#include "params_manage.h"
 
#define LAST_PAGE_ADDR  0x0807F800
static uint16_t store_Data[1024];

 uint32_t MyFLASH_Readword (uint32_t Address)
{
    return *( ( uint32_t *)(Address) );
}
 
//注意：地址为小端存储
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
 

//上电调用初始化函数，然后就可以加载之前存储在flash中的数据
void store_Init (void)
{
	uint16_t i;
	
    //0x0800Fc00最后一页的首地址；OxA5A5为了判断之前是否存储数据
    if (MyFLASH_ReadHalfword(LAST_PAGE_ADDR)!= 0xA5A5)
    {
        MyFLASH_ErasePage (LAST_PAGE_ADDR);
        MyFIASH_ProgramHalfword(LAST_PAGE_ADDR,0xA5A5);
        // 一页的大小是2K,存储数据数组大小是半字。第一位存储标志位，后面数据位清0
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
 
//调用保存函数就会将数据写入flash存储器中
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
