#include <avr/io.h>
#include <error_codes.h>


/**
 * @brief Initialization function
 * 
 */
void init(void);


/**
 * @brief Main loop function
 * 
 */
void loop(void);


int main(void)
{
    init();

    while (1) loop();

    return SUCCESS;
}


void init(void)
{

}

void loop(void)
{

}