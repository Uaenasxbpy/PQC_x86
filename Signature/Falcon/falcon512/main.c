#include <stdio.h>
#include <string.h>
#include "api.h"

int main(){
    // 定义密钥、签名、公钥和消息
    unsigned char pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char sk[CRYPTO_SECRETKEYBYTES];
    // 签名
    unsigned char sm[CRYPTO_BYTES]; 
    // 签名的消息
    unsigned char messgae[] = "My name is XB, from bupt.";
    // 签名消息长度
    unsigned long long messagelen = strlen((const char *)messgae);
    // 签名后的消息长度
    unsigned long long smlen;

    // 生成公钥和私钥
    crypto_sign_keypair(pk, sk);

    // 使用私钥对消息进行签名
    crypto_sign(sm, &smlen, messgae, messagelen, sk);

    // 使用公钥验证签名
    unsigned char m[messagelen];
    unsigned long long mlen;
    if (crypto_sign_open(m, &mlen, sm, smlen, pk) != 0) {
        printf("Signature verification failed!\n");
        return -1;
    } else {
        printf("Signature verification succeeded!\n");
        printf("Original message: %s\n", messgae);
        printf("Verified message: %s\n\n", m);
    }   

    return 0;
}