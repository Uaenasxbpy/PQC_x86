#include <stdio.h>
#include "api.h"
#include "parameters.h"

int main() {

	printf("\n");
	printf("*********************\n");
	printf("**** HQC-%d-%d ****\n", PARAM_SECURITY, PARAM_DFR_EXP);
	printf("*********************\n");
	// 参数
	printf("\n");
	printf("N: %d   ", PARAM_N);
	printf("N1: %d   ", PARAM_N1);
	printf("N2: %d   ", PARAM_N2);
	printf("OMEGA: %d   ", PARAM_OMEGA);
	printf("OMEGA_R: %d   ", PARAM_OMEGA_R);
	printf("Failure rate: 2^-%d   ", PARAM_DFR_EXP);
	printf("Sec: %d bits", PARAM_SECURITY);
	printf("\n");

	// 公钥、私钥、密文、共享密钥
	unsigned char pk[PUBLIC_KEY_BYTES];
	unsigned char sk[SECRET_KEY_BYTES];
	unsigned char ct[CIPHERTEXT_BYTES];
	unsigned char key1[SHARED_SECRET_BYTES];
	unsigned char key2[SHARED_SECRET_BYTES];

	// 密钥协商阶段
	crypto_kem_keypair(pk, sk);
	crypto_kem_enc(ct, key1, pk);
	crypto_kem_dec(key2, ct, sk);

	printf("\n\nsecret1: ");
	for(int i = 0 ; i < SHARED_SECRET_BYTES ; ++i) printf("%x", key1[i]);

	printf("\nsecret2: ");
	for(int i = 0 ; i < SHARED_SECRET_BYTES ; ++i) printf("%x", key2[i]);
	printf("\n");
	// 如果key1和key2相等，说明密钥协商成功
	int flag = 1;
	for(int i = 0 ; i < SHARED_SECRET_BYTES ; ++i){
		if(key1[i] != key2[i]){
			flag = 0;
			break;
		}
	}
	if (flag){
		printf("Share key sucess!\n");
		printf("Key: ");
		for(int i = 0 ; i < SHARED_SECRET_BYTES ; ++i){
			printf("%x", key1[i]);
		} 
		printf("\n");
	} else{
		printf("Falid to share key!\n");
	}
	return 0;
}
