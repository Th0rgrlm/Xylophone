#include "./uart.h"
#include <windows.h>
#include "./error_codes.h"

HANDLE h_serial;

uint32_t get_file_len(FILE* file);
int16_t uart_init(uint8_t COM);
int16_t uart_close(void);
int16_t uart_send_file(FILE* file, uint64_t file_len, uint16_t chunk);



int16_t uart_send(FILE* parsed, uint8_t COM)
{
    int16_t result;
    if ((result = uart_init(COM)) != SUCCESS) return result;
    uint32_t file_len = get_file_len(parsed);

    
    if ((result = uart_send_file(parsed, file_len, 512)) != SUCCESS) return result;

    uart_close();
}

uint32_t get_file_len(FILE* file)
{
    fseek(file, 0, SEEK_END);
    uint32_t len = ftell(file);
    rewind(file);
    return len;
}


int16_t uart_init(uint8_t COM)
{
    DCB dcb_serial_params = {0};
    COMMTIMEOUTS timeouts = {0};

    printf("Opening serial port...\n");
    uint8_t file_path[16];
    snprintf(file_path, 16, "\\\\.\\COM%d", COM); // Create path
    h_serial = CreateFile(file_path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); // Try to create COM file

    if (h_serial == INVALID_HANDLE_VALUE) // Check for successfull creation
    {
        fprintf(stderr, "Failed to open serial port %d\n", COM);
        return ERROR_COM_OPEN;
    }

    dcb_serial_params.DCBlength = sizeof(dcb_serial_params); // Assign length
    if (GetCommState(h_serial, &dcb_serial_params) == 0)
    {
        fprintf(stderr, "Failed to get device status\n");
        CloseHandle(h_serial);
        return ERROR_COM_STATUS;
    }

    dcb_serial_params.BaudRate = CBR_115200; // Set baudrate
    dcb_serial_params.ByteSize = 8; // Data bits per byte sent
    dcb_serial_params.StopBits = ONESTOPBIT; // Number of stop bits
    dcb_serial_params.Parity = NOPARITY; // No parity bit

    if (SetCommState(h_serial, &dcb_serial_params) == 0) // Set comm state
    {
        fprintf(stderr, "Failed to set device status\n");
        CloseHandle(h_serial);
        return ERROR_COM_STATUS;
    }

    timeouts.ReadIntervalTimeout = 5000;
    timeouts.ReadTotalTimeoutConstant = 5000;
    timeouts.ReadTotalTimeoutMultiplier = 1000;
    timeouts.WriteTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (SetCommTimeouts(h_serial, &timeouts) == 0)
    {
        fprintf(stderr, "Failed to assign timeouts\n");
        CloseHandle(h_serial);
        return ERROR_COM_STATUS;
    }
    return SUCCESS;
}

int16_t uart_close(void)
{
    printf("Closing serial port...\n");
    if (CloseHandle(h_serial) == 0)
    {
        fprintf(stderr, "Error closing serial port\n");
        return ERROR_COM_EXIT;
    }
    return SUCCESS;
}

int16_t uart_send_file(FILE* file, uint64_t file_len, uint16_t chunk)
{
    uint8_t max_chunk = file_len / chunk + 1; // Get maximum chunks to send
    uint8_t buffer[chunk];
    memset(buffer, 0x00, chunk); // Clear buffer
    for (int i = 0; i < max_chunk; i++)
    {
        uint16_t size = chunk; // Set size to be sent to chunk size
        if (i == max_chunk - 1) // On last chunk
        {
            size = file_len % chunk; // Set size to the remainder of the file
        }
        if (fread(buffer, 1, size, file) != size) // Read bytes from file
        {
            fprintf(stderr, "Error getting bytes from file - incorrect size\n");
            return ERROR_COM_WRITE;
        }

        DWORD bytes_written, total_bytes_written = 0;
        printf("Sending chunk %i...\n", i);

        if (!WriteFile(h_serial, buffer, size, &bytes_written, NULL)) // Send bytes
        {
            fprintf(stderr, "Error sending chunk %i\n", i);
            CloseHandle(h_serial);
            return ERROR_COM_WRITE;
        }

        char c;
        DWORD bytes_read = 0;
        BOOL bOk = ReadFile(h_serial, &c, 1, &bytes_read, NULL);
        if (!bOk || bytes_read <= 0)
        {
            fprintf(stderr, "Error reading from serial port, closing\n");
            CloseHandle(h_serial);
            return ERROR_COM_TIMEOUT;
        }
        if (c == 'C') continue; // Continue
        else if (c == 'A') break; // Abort
    }
}





