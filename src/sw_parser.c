#include <sw_parser.h>
#include <str_utils.h>

#include <stdlib.h>
#include <string.h>

struct StatusDscr {
    uint8_t     sw1;
    char*       sw2;
    char        type;
    char*       msg;
};

char text[512];

// static struct StatusDscr stats[] = {
//     {
//     .sw1 = 0x60,
//     .sw2 = "None",
//     .type = 'E',
//     .msg = "Class not supported",
//     }
// };

static struct StatusDscr stats[] = {
    {.sw1 = 0X6,   .sw2 = "None",.type = 'E',  .msg = "Class not supported."},
    {.sw1 = 0X61,  .sw2 = "None",.type = 'I',  .msg = "Response bytes still available"},
    {.sw1 = 0X61,  .sw2 = "XX",  .type = 'I',  .msg = "Command successfully executed; {} bytes of data are available and can be requested using GET RESPONSE."},
    {.sw1 = 0X62,  .sw2 = "None",.type = 'W',  .msg = "State of non-volatile memory unchanged"},
    {.sw1 = 0X62,  .sw2 = "00",  .type = 'W',  .msg = "No information given (NV-Ram not changed)"},
    {.sw1 = 0X62,  .sw2 = "01",  .type = 'W',  .msg = "NV-Ram not changed 1."},
    {.sw1 = 0X62,  .sw2 = "81",  .type = 'W',  .msg = "Part of returned data may be corrupted"},
    {.sw1 = 0X62,  .sw2 = "82",  .type = 'W',  .msg = "End of file/record reached before reading Le bytes"},
    {.sw1 = 0X62,  .sw2 = "83",  .type = 'W',  .msg = "Selected file invalidated"},
    {.sw1 = 0X62,  .sw2 = "84",  .type = 'W',  .msg = "Selected file is not valid. FCI not formated according to ISO"},
    {.sw1 = 0X62,  .sw2 = "85",  .type = 'W',  .msg = "No input data available from a sensor on the card. No Purse Engine enslaved for R3bc"},
    {.sw1 = 0X62,  .sw2 = "A2",  .type = 'W',  .msg = "Wrong R-MAC"},
    {.sw1 = 0X62,  .sw2 = "A4",  .type = 'W',  .msg = "Card locked (during reset( ))"},
    {.sw1 = 0X62,  .sw2 = "CX",  .type = 'W',  .msg = "Counter with value x (command dependent)"},
    {.sw1 = 0X62,  .sw2 = "F1",  .type = 'W',  .msg = "Wrong C-MAC"},
    {.sw1 = 0X62,  .sw2 = "F3",  .type = 'W',  .msg = "Internal reset"},
    {.sw1 = 0X62,  .sw2 = "F5",  .type = 'W',  .msg = "Default agent locked"},
    {.sw1 = 0X62,  .sw2 = "F7",  .type = 'W',  .msg = "Cardholder locked"},
    {.sw1 = 0X62,  .sw2 = "F8",  .type = 'W',  .msg = "Basement is current agent"},
    {.sw1 = 0X62,  .sw2 = "F9",  .type = 'W',  .msg = "CALC Key Set not unblocked"},
    {.sw1 = 0X62,  .sw2 = "FX",  .type = 'W',  .msg = "-"},
    {.sw1 = 0X62,  .sw2 = "XX",  .type = 'W',  .msg = "RFU"},
    {.sw1 = 0X63,  .sw2 = "None",.type = 'W',  .msg = "State of non-volatile memory changed"},
    {.sw1 = 0X63,  .sw2 = "00",  .type = 'W',  .msg = "No information given (NV-Ram changed)"},
    {.sw1 = 0X63,  .sw2 = "81",  .type = 'W',  .msg = "File filled up by the last write. Loading/updating is not allowed."},
    {.sw1 = 0X63,  .sw2 = "82",  .type = 'W',  .msg = "Card key not supported."},
    {.sw1 = 0X63,  .sw2 = "83",  .type = 'W',  .msg = "Reader key not supported."},
    {.sw1 = 0X63,  .sw2 = "84",  .type = 'W',  .msg = "Plaintext transmission not supported."},
    {.sw1 = 0X63,  .sw2 = "85",  .type = 'W',  .msg = "Secured transmission not supported."},
    {.sw1 = 0X63,  .sw2 = "86",  .type = 'W',  .msg = "Volatile memory is not available."},
    {.sw1 = 0X63,  .sw2 = "87",  .type = 'W',  .msg = "Non-volatile memory is not available."},
    {.sw1 = 0X63,  .sw2 = "88",  .type = 'W',  .msg = "Key number not valid."},
    {.sw1 = 0X63,  .sw2 = "89",  .type = 'W',  .msg = "Key length is not correct."},
    {.sw1 = 0X63,  .sw2 = "C0",  .type = 'W',  .msg = "Verify fail, no try left."},
    {.sw1 = 0X63,  .sw2 = "C1",  .type = 'W',  .msg = "Verify fail, 1 try left."},
    {.sw1 = 0X63,  .sw2 = "C2",  .type = 'W',  .msg = "Verify fail, 2 tries left."},
    {.sw1 = 0X63,  .sw2 = "C3",  .type = 'W',  .msg = "Verify fail, 3 tries left."},
    {.sw1 = 0X63,  .sw2 = "CX",  .type = 'W',  .msg = "The counter has reached the value ‘x’ (0 = x = 15) (command dependent)."},
    {.sw1 = 0X63,  .sw2 = "F1",  .type = 'W',  .msg = "More data expected."},
    {.sw1 = 0X63,  .sw2 = "F2",  .type = 'W',  .msg = "More data expected and proactive command pending."},
    {.sw1 = 0X63,  .sw2 = "FX",  .type = 'W',  .msg = "-"},
    {.sw1 = 0X63,  .sw2 = "XX",  .type = 'W',  .msg = "RFU"},
    {.sw1 = 0X64,  .sw2 = "None",.type = 'E',  .msg = "State of non-volatile memory unchanged"},
    {.sw1 = 0X64,  .sw2 = "00",  .type = 'E',  .msg = "No information given (NV-Ram not changed)"},
    {.sw1 = 0X64,  .sw2 = "01",  .type = 'E',  .msg = "Command timeout. Immediate response required by the card."},
    {.sw1 = 0X64,  .sw2 = "XX",  .type = 'E',  .msg = "RFU"},
    {.sw1 = 0X65,  .sw2 = "None",.type = 'E',  .msg = "State of non-volatile memory changed"},
    {.sw1 = 0X65,  .sw2 = "00",  .type = 'E',  .msg = "No information given"},
    {.sw1 = 0X65,  .sw2 = "01",  .type = 'E',  .msg = "Write error. Memory failure. There have been problems in writing or reading the EEPROM. Other hardware problems may also bring this error."},
    {.sw1 = 0X65,  .sw2 = "81",  .type = 'E',  .msg = "Memory failure"},
    {.sw1 = 0X65,  .sw2 = "FX",  .type = 'E',  .msg = "-"},
    {.sw1 = 0X65,  .sw2 = "XX",  .type = 'E',  .msg = "RFU"},
    {.sw1 = 0X66,  .sw2 = "None",.type = 'S',  .msg = ""},
    {.sw1 = 0X66,  .sw2 = "00",  .type = 'S',  .msg = "Error while receiving (timeout)"},
    {.sw1 = 0X66,  .sw2 = "01",  .type = 'S',  .msg = "Error while receiving (character parity error)"},
    {.sw1 = 0X66,  .sw2 = "02",  .type = 'S',  .msg = "Wrong checksum"},
    {.sw1 = 0X66,  .sw2 = "03",  .type = 'S',  .msg = "The current DF file without FCI"},
    {.sw1 = 0X66,  .sw2 = "04",  .type = 'S',  .msg = "No SF or KF under the current DF"},
    {.sw1 = 0X66,  .sw2 = "69",  .type = 'S',  .msg = "Incorrect Encryption/Decryption Padding"},
    {.sw1 = 0X66,  .sw2 = "XX",  .type = 'S',  .msg = "-"},
    {.sw1 = 0X67,  .sw2 = "None",.type = 'E',  .msg = ""},
    {.sw1 = 0X67,  .sw2 = "00",  .type = 'E',  .msg = "Wrong length"},
    {.sw1 = 0X67,  .sw2 = "XX",  .type = 'E',  .msg = "length incorrect (procedure)(ISO 7816-3)"},
    {.sw1 = 0X68,  .sw2 = "None",.type = 'E',  .msg = "Functions in CLA not supported"},
    {.sw1 = 0X68,  .sw2 = "00",  .type = 'E',  .msg = "No information given (The request function is not supported by the card)"},
    {.sw1 = 0X68,  .sw2 = "81",  .type = 'E',  .msg = "Logical channel not supported"},
    {.sw1 = 0X68,  .sw2 = "82",  .type = 'E',  .msg = "Secure messaging not supported"},
    {.sw1 = 0X68,  .sw2 = "83",  .type = 'E',  .msg = "Last command of the chain expected"},
    {.sw1 = 0X68,  .sw2 = "84",  .type = 'E',  .msg = "Command chaining not supported"},
    {.sw1 = 0X68,  .sw2 = "FX",  .type = 'E',  .msg = "-"},
    {.sw1 = 0X68,  .sw2 = "XX",  .type = 'E',  .msg = "RFU"},
    {.sw1 = 0X69,  .sw2 = "None",.type = 'E',  .msg = "Command not allowed"},
    {.sw1 = 0X69,  .sw2 = "00",  .type = 'E',  .msg = "No information given (Command not allowed)"},
    {.sw1 = 0X69,  .sw2 = "01",  .type = 'E',  .msg = "Command not accepted (inactive state)"},
    {.sw1 = 0X69,  .sw2 = "81",  .type = 'E',  .msg = "Command incompatible with file structure"},
    {.sw1 = 0X69,  .sw2 = "82",  .type = 'E',  .msg = "Security condition not satisfied."},
    {.sw1 = 0X69,  .sw2 = "83",  .type = 'E',  .msg = "Authentication method blocked"},
    {.sw1 = 0X69,  .sw2 = "84",  .type = 'E',  .msg = "Referenced data reversibly blocked (invalidated)"},
    {.sw1 = 0X69,  .sw2 = "85",  .type = 'E',  .msg = "Conditions of use not satisfied."},
    {.sw1 = 0X69,  .sw2 = "86",  .type = 'E',  .msg = "Command not allowed (no current EF)"},
    {.sw1 = 0X69,  .sw2 = "87",  .type = 'E',  .msg = "Expected secure messaging (SM) object missing"},
    {.sw1 = 0X69,  .sw2 = "88",  .type = 'E',  .msg = "Incorrect secure messaging (SM) data object"},
    {.sw1 = 0X69,  .sw2 = "8D",  .type = ' ',  .msg = "Reserved"},
    {.sw1 = 0X69,  .sw2 = "96",  .type = 'E',  .msg = "Data must be updated again"},
    {.sw1 = 0X69,  .sw2 = "E1",  .type = 'E',  .msg = "POL1 of the currently Enabled Profile prevents this action."},
    {.sw1 = 0X69,  .sw2 = "F0",  .type = 'E',  .msg = "Permission Denied"},
    {.sw1 = 0X69,  .sw2 = "F1",  .type = 'E',  .msg = "Permission Denied - Missing Privilege"},
    {.sw1 = 0X69,  .sw2 = "FX",  .type = 'E',  .msg = "-"},
    {.sw1 = 0X69,  .sw2 = "XX",  .type = 'E',  .msg = "RFU"},
    {.sw1 = 0X6A,  .sw2 = "None",.type = 'E',  .msg = "Wrong parameter(s) P1-P2"},
    {.sw1 = 0X6A,  .sw2 = "00",  .type = 'E',  .msg = "No information given (Bytes P1 and/or P2 are incorrect)"},
    {.sw1 = 0X6A,  .sw2 = "80",  .type = 'E',  .msg = "The parameters in the data field are incorrect."},
    {.sw1 = 0X6A,  .sw2 = "81",  .type = 'E',  .msg = "Function not supported"},
    {.sw1 = 0X6A,  .sw2 = "82",  .type = 'E',  .msg = "File not found"},
    {.sw1 = 0X6A,  .sw2 = "83",  .type = 'E',  .msg = "Record not found"},
    {.sw1 = 0X6A,  .sw2 = "84",  .type = 'E',  .msg = "There is insufficient memory space in record or file"},
    {.sw1 = 0X6A,  .sw2 = "85",  .type = 'E',  .msg = "Lc inconsistent with TLV structure"},
    {.sw1 = 0X6A,  .sw2 = "86",  .type = 'E',  .msg = "Incorrect P1 or P2 parameter."},
    {.sw1 = 0X6A,  .sw2 = "87",  .type = 'E',  .msg = "Lc inconsistent with P1-P2"},
    {.sw1 = 0X6A,  .sw2 = "88",  .type = 'E',  .msg = "Referenced data not found"},
    {.sw1 = 0X6A,  .sw2 = "89",  .type = 'E',  .msg = "File already exists"},
    {.sw1 = 0X6A,  .sw2 = "8A",  .type = 'E',  .msg = "DF name already exists."},
    {.sw1 = 0X6A,  .sw2 = "F0",  .type = 'E',  .msg = "Wrong parameter value"},
    {.sw1 = 0X6A,  .sw2 = "FX",  .type = 'E',  .msg = "-"},
    {.sw1 = 0X6A,  .sw2 = "XX",  .type = 'E',  .msg = "RFU"},
    {.sw1 = 0X6B,  .sw2 = "None",.type = 'E',  .msg = ""},
    {.sw1 = 0X6B,  .sw2 = "00",  .type = 'E',  .msg = "Wrong parameter(s) P1-P2"},
    {.sw1 = 0X6B,  .sw2 = "XX",  .type = 'E',  .msg = "Reference incorrect (procedure byte), (ISO 7816-3)"},
    {.sw1 = 0X6C,  .sw2 = "None",.type = 'E',  .msg = "Wrong length Le"},
    {.sw1 = 0X6C,  .sw2 = "00",  .type = 'E',  .msg = "Incorrect P3 length."},
    {.sw1 = 0X6C,  .sw2 = "XX",  .type = 'E',  .msg = "Bad length value in Le; {} is the correct exact Le"},
    {.sw1 = 0X6D,  .sw2 = "None",.type = 'E',  .msg = ""},
    {.sw1 = 0X6D,  .sw2 = "00",  .type = 'E',  .msg = "Instruction code not supported or invalid"},
    {.sw1 = 0X6D,  .sw2 = "XX",  .type = 'E',  .msg = "Instruction code not programmed or invalid (procedure byte), (ISO 7816-3)"},
    {.sw1 = 0X6E,  .sw2 = "None",.type = 'E',  .msg = ""},
    {.sw1 = 0X6E,  .sw2 = "00",  .type = 'E',  .msg = "Class not supported"},
    {.sw1 = 0X6E,  .sw2 = "XX",  .type = 'E',  .msg = "Instruction class not supported (procedure byte), (ISO 7816-3)"},
    {.sw1 = 0X6F,  .sw2 = "None",.type = 'E',  .msg = "Internal exception"},
    {.sw1 = 0X6F,  .sw2 = "00",  .type = 'E',  .msg = "Command aborted - more exact diagnosis not possible (e.g., operating system error)."},
    {.sw1 = 0X6F,  .sw2 = "FF",  .type = 'E',  .msg = "Card dead (overuse, …)"},
    {.sw1 = 0X6F,  .sw2 = "XX",  .type = 'E',  .msg = "No precise diagnosis (procedure byte), (ISO 7816-3)"},
    {.sw1 = 0X9,   .sw2 = "None",.type = ' ',  .msg =  ""},
    {.sw1 = 0X90,  .sw2 = "00",  .type = 'I',  .msg = "Command successfully executed (OK)."},
    {.sw1 = 0X90,  .sw2 = "04",  .type = 'W',  .msg = "PIN not succesfully verified, 3 or more PIN tries left"},
    {.sw1 = 0X90,  .sw2 = "08",  .type = ' ',  .msg =  "Key,/file not found"},
    {.sw1 = 0X90,  .sw2 = "80",  .type = 'W',  .msg = "Unblock Try Counter has reached zero"},
    {.sw1 = 0X91,  .sw2 = "00",  .type = ' ',  .msg =  "OK,"},
    {.sw1 = 0X91,  .sw2 = "01",  .type = ' ',  .msg =  "States,.activity, States.lock Status or States.lockable has wrong value"},
    {.sw1 = 0X91,  .sw2 = "02",  .type = ' ',  .msg =  "Transaction, number reached its limit"},
    {.sw1 = 0X91,  .sw2 = "0C",  .type = ' ',  .msg =  "No, changes"},
    {.sw1 = 0X91,  .sw2 = "0E",  .type = ' ',  .msg =  "Insufficient, NV-Memory to complete command"},
    {.sw1 = 0X91,  .sw2 = "1C",  .type = ' ',  .msg =  "Command, code not supported"},
    {.sw1 = 0X91,  .sw2 = "1E",  .type = ' ',  .msg =  "CRC, or MAC does not match data"},
    {.sw1 = 0X91,  .sw2 = "40",  .type = ' ',  .msg =  "Invalid, key number specified"},
    {.sw1 = 0X91,  .sw2 = "7E",  .type = ' ',  .msg =  "Length, of command string invalid"},
    {.sw1 = 0X91,  .sw2 = "9D",  .type = ' ',  .msg =  "Not, allow the requested command"},
    {.sw1 = 0X91,  .sw2 = "9E",  .type = ' ',  .msg =  "Value, of the parameter invalid"},
    {.sw1 = 0X91,  .sw2 = "A0",  .type = ' ',  .msg =  "Requested, AID not present on PICC"},
    {.sw1 = 0X91,  .sw2 = "A1",  .type = ' ',  .msg =  "Unrecoverable, error within application"},
    {.sw1 = 0X91,  .sw2 = "AE",  .type = ' ',  .msg =  "Authentication, status does not allow the requested command"},
    {.sw1 = 0X91,  .sw2 = "AF",  .type = ' ',  .msg =  "Additional, data frame is expected to be sent"},
    {.sw1 = 0X91,  .sw2 = "BE",  .type = ' ',  .msg =  "Out, of boundary"},
    {.sw1 = 0X91,  .sw2 = "C1",  .type = ' ',  .msg =  "Unrecoverable, error within PICC"},
    {.sw1 = 0X91,  .sw2 = "CA",  .type = ' ',  .msg =  "Previous, Command was not fully completed"},
    {.sw1 = 0X91,  .sw2 = "CD",  .type = ' ',  .msg =  "PICC, was disabled by an unrecoverable error"},
    {.sw1 = 0X91,  .sw2 = "CE",  .type = ' ',  .msg =  "Number, of Applications limited to 28"},
    {.sw1 = 0X91,  .sw2 = "DE",  .type = ' ',  .msg =  "File, or application already exists"},
    {.sw1 = 0X91,  .sw2 = "EE",  .type = ' ',  .msg =  "Could, not complete NV-write operation due to loss of power"},
    {.sw1 = 0X91,  .sw2 = "F0",  .type = ' ',  .msg =  "Specified, file number does not exist"},
    {.sw1 = 0X91,  .sw2 = "F1",  .type = ' ',  .msg =  "Unrecoverable, error within file"},
    {.sw1 = 0X92,  .sw2 = "0x",  .type = 'I',  .msg = "Writing to EEPROM successful after ‘x’ attempts."},
    {.sw1 = 0X92,  .sw2 = "10",  .type = 'E',  .msg = "Insufficient memory. No more storage available."},
    {.sw1 = 0X92,  .sw2 = "40",  .type = 'E',  .msg = "Writing to EEPROM not successful."},
    {.sw1 = 0X93,  .sw2 = "01",  .type = ' ',  .msg =  "Integrity, error"},
    {.sw1 = 0X93,  .sw2 = "02",  .type = ' ',  .msg =  "Candidate, S2 invalid"},
    {.sw1 = 0X93,  .sw2 = "03",  .type = 'E',  .msg = "Application is permanently locked"},
    {.sw1 = 0X94,  .sw2 = "00",  .type = 'E',  .msg = "No EF selected."},
    {.sw1 = 0X94,  .sw2 = "01",  .type = ' ',  .msg = "Candidate, currency code does not match purse currency"},
    {.sw1 = 0X94,  .sw2 = "02",  .type = ' ',  .msg =  "Candidate, amount too high"},
    {.sw1 = 0X94,  .sw2 = "02",  .type = 'E',  .msg = "Address range exceeded."},
    {.sw1 = 0X94,  .sw2 = "03",  .type = ' ',  .msg =  "Candidate, amount too low"},
    {.sw1 = 0X94,  .sw2 = "04",  .type = 'E',  .msg = "FID not found, record not found or comparison pattern not found."},
    {.sw1 = 0X94,  .sw2 = "05",  .type = ' ',  .msg =  "Problems, in the data field"},
    {.sw1 = 0X94,  .sw2 = "06",  .type = 'E',  .msg = "Required MAC unavailable"},
    {.sw1 = 0X94,  .sw2 = "07",  .type = ' ',  .msg =  "Bad, currency : purse engine has no slot with R3bc currency"},
    {.sw1 = 0X94,  .sw2 = "08",  .type = ' ',  .msg =  "R3bc, currency not supported in purse engine"},
    {.sw1 = 0X94,  .sw2 = "08",  .type = 'E',  .msg = "Selected file type does not match command."},
    {.sw1 = 0X95,  .sw2 = "80",  .type = ' ',  .msg =  "Bad, sequence"},
    {.sw1 = 0X96,  .sw2 = "81",  .type = ' ',  .msg =  "Slave, not found"},
    {.sw1 = 0X97,  .sw2 = "00",  .type = ' ',  .msg =  "PIN, blocked and Unblock Try Counter is 1 or 2"},
    {.sw1 = 0X97,  .sw2 = "02",  .type = ' ',  .msg =  "Main, keys are blocked"},
    {.sw1 = 0X97,  .sw2 = "04",  .type = ' ',  .msg =  "PIN, not succesfully verified, 3 or more PIN tries left"},
    {.sw1 = 0X97,  .sw2 = "84",  .type = ' ',  .msg =  "Base, key"},
    {.sw1 = 0X97,  .sw2 = "85",  .type = ' ',  .msg =  "Limit, exceeded - C-MAC key"},
    {.sw1 = 0X97,  .sw2 = "86",  .type = ' ',  .msg =  "SM, error - Limit exceeded - R-MAC key"},
    {.sw1 = 0X97,  .sw2 = "87",  .type = ' ',  .msg =  "Limit, exceeded - sequence counter"},
    {.sw1 = 0X97,  .sw2 = "88",  .type = ' ',  .msg =  "Limit, exceeded - R-MAC length"},
    {.sw1 = 0X97,  .sw2 = "89",  .type = ' ',  .msg =  "Service, not available"},
    {.sw1 = 0X98,  .sw2 = "02",  .type = 'E',  .msg = "No PIN defined."},
    {.sw1 = 0X98,  .sw2 = "04",  .type = 'E',  .msg = "Access conditions not satisfied, authentication failed."},
    {.sw1 = 0X98,  .sw2 = "35",  .type = 'E',  .msg = "ASK RANDOM or GIVE RANDOM not executed."},
    {.sw1 = 0X98,  .sw2 = "40",  .type = 'E',  .msg = "PIN verification not successful."},
    {.sw1 = 0X98,  .sw2 = "50",  .type = 'E',  .msg = "INCREASE or DECREASE could not be executed because a limit has been reached."},
    {.sw1 = 0X98,  .sw2 = "62",  .type = 'E',  .msg = "Authentication Error, application specific (incorrect MAC)"},
    {.sw1 = 0X99,  .sw2 = "00",  .type = ' ',  .msg = "1, PIN try left"},
    {.sw1 = 0X99,  .sw2 = "04",  .type = ' ',  .msg = "PIN, not succesfully verified, 1 PIN try left"},
    {.sw1 = 0X99,  .sw2 = "85",  .type = ' ',  .msg = "Wrong, status - Cardholder lock"},
    {.sw1 = 0X99,  .sw2 = "86",  .type = 'E',  .msg = "Missing privilege"},
    {.sw1 = 0X99,  .sw2 = "87",  .type = ' ',  .msg = "PIN, is not installed"},
    {.sw1 = 0X99,  .sw2 = "88",  .type = ' ',  .msg = "Wrong, status - R-MAC state"},
    {.sw1 = 0X9A,  .sw2 = "00",  .type = ' ',  .msg = "2, PIN try left"},
    {.sw1 = 0X9A,  .sw2 = "04",  .type = ' ',  .msg = "PIN, not succesfully verified, 2 PIN try left"},
    {.sw1 = 0X9A,  .sw2 = "71",  .type = ' ',  .msg = "Wrong, parameter value - Double agent AID"},
    {.sw1 = 0X9A,  .sw2 = "72",  .type = ' ',  .msg = "Wrong, parameter value - Double agent Type"},
    {.sw1 = 0X9D,  .sw2 = "05",  .type = 'E',  .msg = "Incorrect certificate type"},
    {.sw1 = 0X9D,  .sw2 = "07",  .type = 'E',  .msg = "Incorrect session data size"},
    {.sw1 = 0X9D,  .sw2 = "08",  .type = 'E',  .msg = "Incorrect DIR file record size"},
    {.sw1 = 0X9D,  .sw2 = "09",  .type = 'E',  .msg = "Incorrect FCI record size"},
    {.sw1 = 0X9D,  .sw2 = "0A",  .type = 'E',  .msg = "Incorrect code size"},
    {.sw1 = 0X9D,  .sw2 = "10",  .type = 'E',  .msg = "Insufficient memory to load application"},
    {.sw1 = 0X9D,  .sw2 = "11",  .type = 'E',  .msg = "Invalid AID"},
    {.sw1 = 0X9D,  .sw2 = "12",  .type = 'E',  .msg = "Duplicate AID"},
    {.sw1 = 0X9D,  .sw2 = "13",  .type = 'E',  .msg = "Application previously loaded"},
    {.sw1 = 0X9D,  .sw2 = "14",  .type = 'E',  .msg = "Application history list full"},
    {.sw1 = 0X9D,  .sw2 = "15",  .type = 'E',  .msg = "Application not open"},
    {.sw1 = 0X9D,  .sw2 = "17",  .type = 'E',  .msg = "Invalid offset"},
    {.sw1 = 0X9D,  .sw2 = "18",  .type = 'E',  .msg = "Application already loaded"},
    {.sw1 = 0X9D,  .sw2 = "19",  .type = 'E',  .msg = "Invalid certificate"},
    {.sw1 = 0X9D,  .sw2 = "1A",  .type = 'E',  .msg = "Invalid signature"},
    {.sw1 = 0X9D,  .sw2 = "1B",  .type = 'E',  .msg = "Invalid KTU"},
    {.sw1 = 0X9D,  .sw2 = "1D",  .type = 'E',  .msg = "MSM controls not set"},
    {.sw1 = 0X9D,  .sw2 = "1E",  .type = 'E',  .msg = "Application signature does not exist"},
    {.sw1 = 0X9D,  .sw2 = "1F",  .type = 'E',  .msg = "KTU does not exist"},
    {.sw1 = 0X9D,  .sw2 = "20",  .type = 'E',  .msg = "Application not loaded"},
    {.sw1 = 0X9D,  .sw2 = "21",  .type = 'E',  .msg = "Invalid Open command data length"},
    {.sw1 = 0X9D,  .sw2 = "30",  .type = 'E',  .msg = "Check data parameter is incorrect (invalid start address)"},
    {.sw1 = 0X9D,  .sw2 = "31",  .type = 'E',  .msg = "Check data parameter is incorrect (invalid length)"},
    {.sw1 = 0X9D,  .sw2 = "32",  .type = 'E',  .msg = "Check data parameter is incorrect (illegal memory check area)"},
    {.sw1 = 0X9D,  .sw2 = "40",  .type = 'E',  .msg = "Invalid MSM Controls ciphertext"},
    {.sw1 = 0X9D,  .sw2 = "41",  .type = 'E',  .msg = "MSM controls already set"},
    {.sw1 = 0X9D,  .sw2 = "42",  .type = 'E',  .msg = "Set MSM Controls data length less than 2 bytes"},
    {.sw1 = 0X9D,  .sw2 = "43",  .type = 'E',  .msg = "Invalid MSM Controls data length"},
    {.sw1 = 0X9D,  .sw2 = "44",  .type = 'E',  .msg = "Excess MSM Controls ciphertext"},
    {.sw1 = 0X9D,  .sw2 = "45",  .type = 'E',  .msg = "Verification of MSM Controls data failed"},
    {.sw1 = 0X9D,  .sw2 = "50",  .type = 'E',  .msg = "Invalid MCD Issuer production ID"},
    {.sw1 = 0X9D,  .sw2 = "51",  .type = 'E',  .msg = "Invalid MCD Issuer ID"},
    {.sw1 = 0X9D,  .sw2 = "52",  .type = 'E',  .msg = "Invalid set MSM controls data date"},
    {.sw1 = 0X9D,  .sw2 = "53",  .type = 'E',  .msg = "Invalid MCD number"},
    {.sw1 = 0X9D,  .sw2 = "54",  .type = 'E',  .msg = "Reserved field error"},
    {.sw1 = 0X9D,  .sw2 = "55",  .type = 'E',  .msg = "Reserved field error"},
    {.sw1 = 0X9D,  .sw2 = "56",  .type = 'E',  .msg = "Reserved field error"},
    {.sw1 = 0X9D,  .sw2 = "57",  .type = 'E',  .msg = "Reserved field error"},
    {.sw1 = 0X9D,  .sw2 = "60",  .type = 'E',  .msg = "MAC verification failed"},
    {.sw1 = 0X9D,  .sw2 = "61",  .type = 'E',  .msg = "Maximum number of unblocks reached"},
    {.sw1 = 0X9D,  .sw2 = "62",  .type = 'E',  .msg = "Card was not blocked"},
    {.sw1 = 0X9D,  .sw2 = "63",  .type = 'E',  .msg = "Crypto functions not available"},
    {.sw1 = 0X9D,  .sw2 = "64",  .type = 'E',  .msg = "No application loaded"},
    {.sw1 = 0X9E,  .sw2 = "00",  .type = ' ',  .msg =  "PIN, not installed"},
    {.sw1 = 0X9E,  .sw2 = "04",  .type = ' ',  .msg =  "PIN, not succesfully verified, PIN not installed"},
    {.sw1 = 0X9F,  .sw2 = "00",  .type = ' ',  .msg =     "PIN, blocked and Unblock Try Counter is 3"},
    {.sw1 = 0X9F,  .sw2 = "04",  .type = ' ',  .msg =  "PIN, not succesfully verified, PIN blocked and Unblock Try Counter is 3"},
    {.sw1 = 0X9F,  .sw2 = "XX",  .type = ' ',  .msg =     "Command, successfully executed; {} bytes of data are available and can be requested using GET RESPONSE."},
    /* {.sw1 = status_dscr(0    .sw2 = ,	"XX",   '.' y .msg = e = "",     "Application, related status, (ISO 7816-3)"}, */
};

static bool sw2_equal(char* sw2_str, uint8_t sw2_num) {
    uint8_t tmp = 0;

    if (is_digit(sw2_str[0])) {
        tmp += 16 * get_digit(sw2_str[0]);
    } else {
        return false;
    }

    if (is_digit(sw2_str[1])) {
        tmp += get_digit(sw2_str[1]);
    } else {
        return false;
    }

    return tmp == sw2_num;
}

const char* status_to_string(uint8_t sw1, uint8_t  sw2) {
    sprintf(text, "unknonw status: 0x%02X 0x%02X\n", sw1, sw2);

    size_t num_stats = sizeof(stats) / sizeof(stats[0]);
    printf("sizeof(stats) = %zu, sizeof(stats[0]) = %zu\n", sizeof(stats), sizeof(stats[0]));
    printf("num_stats = %zu\n", num_stats);

    for (size_t i = 0; i < num_stats; i++) {
        if (stats[i].sw1 != sw1) {
            continue;
        }

        if (strcmp(stats[i].sw2, "XX") == 0) {
            if (sw1 == 0X61 || sw1 == 0X6C || sw1 == 0x9F) {
                char format[512];
                sprintf(format, "%c: %s", stats[i].type, stats[i].msg);
                sprintf(text, format, sw2);
            } else {
                sprintf(text, "%c: %s", stats[i].type, stats[i].msg);
            }
            break;
        } else {
            if (sw2_equal(stats[i].sw2, sw2)) {
                sprintf(text, "%c: %s", stats[i].type, stats[i].msg);
            }
        }
    }
    return &text[0];
}