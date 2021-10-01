#include "basetype.h"
#include "aesencrypt.h"
#include <openssl/aes.h>

#define VAVA_AES_KEY				"vavalic2"

int VAVA_Aes_Encrypt(char* in, char* out, int size)
{  
	AES_KEY aes; 
	int en_size;
	char aeskey[AES_BLOCK_SIZE];

	if(in == NULL || out == NULL)
	{
		return -1;
	}

	memset(aeskey, 0, AES_BLOCK_SIZE);
	strcpy(aeskey, VAVA_AES_KEY);
	
    if(AES_set_encrypt_key((unsigned char*)aeskey, 128, &aes) < 0)  
    {  
        return -1;  
    } 

	en_size = 0;
	
	//��������ַ���������������AES_BLOCK_SIZE������������Ҫ�ϸ�����
    while(en_size < size && size - en_size >= 16)
    {
        AES_encrypt((unsigned char *)in, (unsigned char *)out, &aes);
        in += AES_BLOCK_SIZE;  
        out += AES_BLOCK_SIZE;  
        en_size += AES_BLOCK_SIZE;  
    }  
	
    return 0;  
}  

int VAVA_Aes_Decrypt(char* in, char* out, int size)  
{  
	AES_KEY aes;
	int en_size;
	char aeskey[AES_BLOCK_SIZE];
	char* porg_out = out;
    if(in == NULL || out == NULL)
	{
		return -1;
	}

	memset(aeskey, 0, AES_BLOCK_SIZE);
	strcpy(aeskey, VAVA_AES_KEY);
	
    if(AES_set_decrypt_key((unsigned char*)aeskey, 128, &aes) < 0)  
    {  
        return -1;  
    }  

	en_size = 0;

	//��������ַ���������������AES_BLOCK_SIZE������������Ҫ�ϸ�����
    while(en_size < size && size - en_size >= 16)  
    {  
        AES_decrypt((unsigned char*)in, (unsigned char*)out, &aes);  
		
        in += AES_BLOCK_SIZE;  
        out += AES_BLOCK_SIZE;  
        en_size += AES_BLOCK_SIZE;  
    }
	printf("VAVA_Aes_Decrypt aeskey:%s, porg_out[0]:%0x\n", aeskey, (uint8_t)porg_out[0]);
    return 0;  
}

int VAVA_Aes_Check_Video(unsigned char *buff, int size)
{
	if(buff == NULL)
	{
		return -1;
	}

	if(buff[0] == 0x27 && buff[1] == 0xbe && buff[2] == 0x9b && buff[3] == 0x5b)
	{
		VAVA_Aes_Decrypt((char *)buff, (char *)buff, size);
	}

	return 0;
}

int VAVA_Aes_Check_Audio(unsigned char *buff, int size)
{
	if(buff == NULL)
	{
		return -1;
	}

	if(buff[0] == 0xFF)
	{
		return 0;
	}
	else
	{
		buff[0] = 0xFF;
		VAVA_Aes_Decrypt((char *)(buff + 1), (char *)(buff + 1), size - 1);
	}

	return 0;
}

