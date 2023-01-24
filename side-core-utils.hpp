#include "lib_rats_wrapper.h"
#include "static-certs.h"
#include <stdlib.h>

#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')

extern std::string privKey;
extern std::string certificate;

void
hex_dump(const char* title, const uint8_t* buf, uint32_t size, uint32_t number)
{
    int i, j;
    if(title) {
        printf("\n\t%s:\n\n", title);
    }

    for(i = 0; i < size; i += number) {
        printf("%08X: ", i);

        for(j = 0; j < number; j++) {
            if(j % 8 == 0) {
                printf(" ");
            }
            if(i + j < size)
                printf("%02X ", buf[i + j]);
            else
                printf("   ");
        }
        printf(" ");

        for(j = 0; j < number; j++) {
            if(i + j < size) {
                printf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        printf("\n");
    }
}


static void
create_cert_files(const char* server_key,
                  const char* server_cert)
{

    // if(privaton_bounds_validate(cap,
    //                             __func__,
    //                             kms_core_lower_bound(),
    //                             kms_core_upper_bound()) == -1)
    //     return;

    std::string file_path_1 = "";
    std::string mode = "w+";

    if(file_path_1.empty()) {
        file_path_1 = "/mongoose_certs/privkey.pem";
    }
    privKey = (file_path_1);

    FILE* file = fopen(file_path_1.c_str(), mode.c_str());

    if(file == NULL) {
        printf("[WASM-CORE] Error in accessing the private key - errno: %d\n", errno);
    }
    else {
        if(strlen(server_key) == 0)
            fwrite(
                dev_server_cert, sizeof(char), strlen(dev_server_cert), file);
        else
            fwrite(server_key, sizeof(char), strlen(server_key), file);

        fclose(file);
    }

    std::string file_path_2 = "";//privaton_fetch_address(cap, PRIVATON_CERTIFICATE);
    if(file_path_2.empty()) {
        file_path_2 = "/mongoose_certs/cert.pem";
    }

    certificate = (file_path_2);
    file = fopen(file_path_2.c_str(), mode.c_str());

    if(file == NULL) {
        printf("[WASM-CORE] Error in accessing the cert - errno: %d\n", errno);
    }
    else {
        if(strlen(server_key) == 0)
            fwrite(
                dev_server_cert, sizeof(char), strlen(dev_server_cert), file);
        else
            fwrite(server_cert, sizeof(char), strlen(server_cert), file);
        fclose(file);
    }

    std::string file_path_3 = ""; //privaton_fetch_address(cap, PRIVATON_CAUTH);

    if(file_path_3.empty()) {
        file_path_3 = "/mongoose_certs/ca.pem";
    }

    file = fopen(file_path_3.c_str(), mode.c_str());


    if(file == NULL) {
        printf("[WASM-CORE] Error in accessing the ca cert - errno: %d\n", errno);
    }
    else {
        fwrite(ca_cert, sizeof(char), strlen(ca_cert), file);
        fclose(file);
    }

}


std::string
get_ra_evidence()
{
    int ret_code = -1;
    char* evidence_json = NULL;

    const char* buffer = "This is a sample.";

    rats_sgx_evidence_t* evidence =
        (rats_sgx_evidence_t*)malloc(sizeof(rats_sgx_evidence_t));
    if(!evidence) {
        printf("ERROR: No memory to allocate.\n");
    }

    int rats_err = librats_collect(&evidence_json, buffer);
    if(rats_err != 0) {
        printf("ERROR: Collect evidence failed, error code: %#x\n", rats_err);
    }

    if(librats_parse_evidence(evidence_json, evidence) != 0) {
        printf("ERROR: Parse evidence failed.\n");
    }

    std::string evidence_str = evidence_json;
    free(evidence_json);
    // You could use these parameters for further verification.
    hex_dump("Quote", evidence->quote, evidence->quote_size, 32);
    hex_dump("User Data", evidence->user_data, SGX_USER_DATA_SIZE, 32);
    hex_dump("MRENCLAVE", evidence->mr_enclave, SGX_MEASUREMENT_SIZE, 32);
    hex_dump("MRSIGNER", evidence->mr_signer, SGX_MEASUREMENT_SIZE, 32);
    printf("\n\tProduct ID:\t\t%u\n", evidence->product_id);
    printf("\tSecurity Version:\t%u\n", evidence->security_version);
    printf("\tAttributes.flags:\t%llu\n", evidence->att_flags);
    printf("\tAttribute.xfrm:\t\t%llu\n", evidence->att_xfrm);

    // rats_err = librats_verify((const char *)evidence_json,
    // evidence->user_data); if (rats_err != 0) {
    //     printf("ERROR: Evidence is not trusted, error code: %#x.\n",
    //     rats_err);
    // }
    return evidence_json;
}
