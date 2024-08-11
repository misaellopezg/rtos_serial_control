/*
Author: Misael Lopez
Description:
The purpose of this project it to get accquainted with RTOS. This project is comprised of three tasks:
readSerialTask - 
writeSerialTask
setBlinkRateTask
*/

#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <limits.h>

/*Define tasks*/
void readSerialTask( void *pvParameters );
void writeSerialTask( void *pvParameters );
void setBlinkRateTask( void *pvParameters );

/*Define queues*/
QueueHandle_t blinkRateQueue;

void setup() 
{
  Serial.begin(115200);
  blinkRateQueue = xQueueCreate(10, sizeof(int));
  Serial.println("Starting!");
  /*Create tasks*/
  xTaskCreate(
    readSerialTask,
    "Read Serial", 
    128,
    NULL,
    2,
    NULL
  );
  xTaskCreate(
    setBlinkRateTask,
    "Set Blink Rate",
    128,
    NULL,
    1,
    NULL
  );
}

void loop() {
  //Not used with FreeRTOS
}

/*
  Read Serial
  Reads incomming serial data using the Serial class. 
  Inputs: N/A
  Outputs: N/A
*/
void readSerialTask(void *pvParameters)
{
  (void) pvParameters;
  for (;;)
  {
    Serial.println("Serial Task");
    String recData;
    bool dataReceived = false;
    while(Serial.available() > 0)
    {
      if (!dataReceived) 
      {
          dataReceived = true;
      }  
      recData+= (char)Serial.read();
    }
    if(dataReceived)
    {
      //If data is received, check to see what command was sent
      //i.e. blink rate

      //Blink rate command
     if(recData.indexOf("blink") == 0)
     {
      String blinkRateStr = recData.substring(5);
      blinkRateStr.trim();
      int blinkRate = blinkRateStr.toInt();
      if (xQueueSend(blinkRateQueue, &blinkRate, portMAX_DELAY) != pdPASS) 
      {
          Serial.println("Failed to send to queue");
      } 
      else 
      {
          Serial.print("Blink rate sent to queue: ");
          Serial.println(blinkRate);
      }
     }
     //Other commands to be added here
    }
    vTaskDelay(250/portTICK_PERIOD_MS);
  }
}

void setBlinkRateTask(void *pvParameters)
{
  (void) pvParameters;
  int blinkRate;
  pinMode(LED_BUILTIN, OUTPUT);  // Set the built-in LED pin as output
  TickType_t xLastWakeTime;
  TickType_t xFrequency = 100 / portTICK_PERIOD_MS;  // Default blink rate

  // Initialize the xLastWakeTime variable with the current time
  xLastWakeTime = xTaskGetTickCount();

  for (;;) 
  {
    // Wait for the next cycle
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    // Check if there is a new blink rate from the queue
    if (xQueueReceive(blinkRateQueue, &blinkRate, 0) == pdPASS) {
      Serial.print("Received blink rate from queue: ");
      Serial.println(blinkRate);
      xFrequency = blinkRate / portTICK_PERIOD_MS;
    }

    // Blink the LED at the current rate
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(xFrequency / 2);  // Half of the period on
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(xFrequency / 2);  // Half of the period off
  }
}
