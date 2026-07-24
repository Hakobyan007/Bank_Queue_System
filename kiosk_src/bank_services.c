#include "../kiosk_headers/bank_services.h"

char* services[AVAILABLE_SERVICE_LIST_LENGTH] = {"Cash Transactions","Deposits","Loans","Account Management","Card Management","Transfers","Currency Exchange"};
char* service_msgs[AVAILABLE_SERVICE_LIST_LENGTH] = {"CMD_NEW_CASH","CMD_NEW_DEPOSIT","CMD_NEW_LOAN","CMD_ACCOUNT_MGMT","CMD_CARD_MGMT","CMD_NEW_TRANSFER","CMD_EXCHANGE"}; 
char services_letters[AVAILABLE_SERVICE_LIST_LENGTH] = {'C','D','L','A','M','T','E'};