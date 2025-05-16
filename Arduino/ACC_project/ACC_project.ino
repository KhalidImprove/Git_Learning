#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 9;
const int BRAKE_PIN = 7;

//Test
//Test...
// Simulated input values
bool TargetSpeedConfirmed= 0;
int targetSpeed = 40;
int SetTargetSpeed =50;
int brakeApplied;
bool CARPresenceApplied;
bool ACCisOn = 0;
bool Fault=0;
uint8_t value = 50;
uint16_t ACCState;

// MCP_CAN instance
MCP_CAN CAN(SPI_CS_PIN);

// Constants for signal values
const byte BrakeDecelRequest_Apply = 16;  // BrakeDecelRequest Apply signal
const byte BrakeDecelRequest_DoNotApply = 0;  // BrakeDecelRequest DoNotApply signal
const byte ACCSTATE_Standby1 = 128;  // ACC Standby State
const byte ACCSTATE_Standby2 = 1;  // ACC Standby State
const byte ACCSTATE_Active1 = 1;  // ACC Standby State
const byte ACCSTATE_Active2 = 0;   // ACC Active State
const byte BrakeSwitch1_Applied = 128;   // ACC Active State


// Constants for txBuf[] indices
const byte txBuf_ACCSTATE = 3;  // Index 3 for ACCSTATE
const byte txBuf_ACCSTATE_Second = 4;
const byte txBuf_BrakeDecelRequest = 0;  // Index 0 for BrakeDecelRequest
const byte txBuf_TargetSpeed = 1; //Khalid //adding Target speed Tx;
const byte rxBuf_BrakeSwitch1_Applied = 0; // Index 0
const byte rxBuf_SetTargetSpeed = 1; // Index 1 ////Khalid //adding Target speed Rx
const byte rxBuf_Fault = 0; // Index 0

void setup() {
  Serial.begin(115200);
  pinMode(BRAKE_PIN, INPUT);

  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("CAN init OK");
  else {
    Serial.println("CAN init FAIL");
    while (1);
  }

  CAN.setMode(MCP_NORMAL);
  delay(20);
}

void loop() {
  byte txBuf[8];
  txBuf[txBuf_TargetSpeed]= targetSpeed;  //Khalid //send of target speed signal

  if (brakeApplied && !CARPresenceApplied ) {
    txBuf[3] = 128; 
    txBuf[4] = 1; //ACCSTATE_Standby;
    txBuf[txBuf_BrakeDecelRequest] = BrakeDecelRequest_Apply; //BrakeDecelRequest_Apply;
    Serial.println("Brake applied, no car present → ACC standby, Brake apply");
  }
  else if (brakeApplied && CARPresenceApplied) {
    txBuf[txBuf_BrakeDecelRequest] = BrakeDecelRequest_Apply; //BrakeDecelRequest_Apply;
    Serial.println("Brake applied, car present → Brake apply only");
  }
  else {
    txBuf[txBuf_BrakeDecelRequest] = BrakeDecelRequest_DoNotApply; //BrakeDecelRequest_DoNotApply;
    Serial.println("Brake not applied → Do not apply decel");
  }

  if (ACCisOn) {
    txBuf[3] = 128; 
    txBuf[4] = 1; //ACCSTATE_Standby;
    Serial.println("ACC on → ACC standby.");
  }
  else if (!ACCisOn)  {
    txBuf[3] = 0; 
    txBuf[4] = 0; //ACCSTATE_off;
    Serial.println("ACC off → ACC off.");
  }

  if ((value < 25 || !ACCisOn) && ACCState == 2){
    Fault = true;
    Serial.println(">> Fault detected!");
    }

    // If fault is active, switch to standby // Khalid // adding condition on speed
    if (((Fault == 1) && ACCisOn)||(value < 25)) {

      txBuf[txBuf_ACCSTATE] = ACCSTATE_Standby1; 
      txBuf[txBuf_ACCSTATE_Second] = ACCSTATE_Standby2; //ACCSTATE_standby;
      ACCState = 3;  // Set to Standby
      Serial.println(">> Switching to ACC Standby Mode due to fault");
    }
  
  if (TargetSpeedConfirmed && targetSpeed >= 25 && ACCisOn && !Fault && !brakeApplied && value >= 25) {    //Khalid , adding brake condition
    txBuf[txBuf_ACCSTATE] = ACCSTATE_Active2; 
    txBuf[txBuf_ACCSTATE_Second] = ACCSTATE_Active1; //ACCSTATE_Active;
    Serial.println("Target speed confirmed and ACC on → ACC active");
  }

   delay(20);

  if (CAN.checkReceive() == CAN_MSGAVAIL) {
  long unsigned int rxId;
  byte len = 8;
  byte rxBuf[8];

  // Read the CAN message
  CAN.readMsgBuf(&rxId, &len, rxBuf);

  if (rxId == 0x500) {
    if ((rxBuf[rxBuf_Fault] == 5)||(rxBuf[rxBuf_Fault] == 7)){     //Khalid adding conditions 
       Fault =1;
    }
    else if ((rxBuf[rxBuf_Fault] == 1)||(rxBuf[rxBuf_Fault] == 3)){

        Fault = 0;
    }

    targetSpeed = rxBuf[rxBuf_SetTargetSpeed];  // Khalid // read Traget_speed_from Sys variable 

    if (rxBuf[0] == 3){          //Khalid , condition to confirm target speed
      TargetSpeedConfirmed = 1;
    }
    else if ((rxBuf[0] == 0) ||(brakeApplied && !CARPresenceApplied))
         {
      TargetSpeedConfirmed = 0;
    }


    if (TargetSpeedConfirmed) {
      Serial.println("TargetSpeed is Confirmed.");
    } else {
      Serial.println("TargetSpeed is not Confirmed.");
    }
    
    if ((rxBuf[0]  == 1)||(rxBuf[0] == 3)||(rxBuf[0] == 5)||(rxBuf[0] == 7))
      {

      ACCisOn = 1 ; 

      }
    else if ((rxBuf[0] == 0) ||(rxBuf[0] == 4)||(rxBuf[0] == 2)||(rxBuf[0] == 6))
      {

      ACCisOn = 0 ; 

      }

    if (ACCisOn) {
      Serial.println("ACC is On.");
    } else {
      Serial.println("ACC is Off.");
    }
  }

  if (rxId == 0x200) {

    brakeApplied = (rxBuf[rxBuf_BrakeSwitch1_Applied] == BrakeSwitch1_Applied); 

    if (brakeApplied) {
      Serial.println("Brake is applied.");
    } else {
      Serial.println("Brake is not applied.");
    }

    CARPresenceApplied = (rxBuf[1] == 1); 

    if (CARPresenceApplied) {
      Serial.println("CARPresence is applied.");
    } else {
      Serial.println("CARPresence is not applied.");
    }
  }

  if (rxId == 0x300) {

      value = rxBuf[0];

      Serial.print("Received Value: ");
      Serial.println(value);

      if (value < 25) {
        txBuf[3] = 128; 
        txBuf[4] = 1; //ACCSTATE_Standby;
        Serial.println("Vehicle speed is less than 25, ACC is standby.");
      }
}
  if (rxId == 0x100) {
      ACCState = (uint16_t)rxBuf[3] | ((uint16_t)rxBuf[4] << 8);
      Serial.println(ACCState);
}
  delay(20);
  // bool brakeApplied = digitalRead(BRAKE_PIN) == LOW;

  // Send CAN message
  byte result = CAN.sendMsgBuf(0x100, 8, 8, txBuf);
  if (result != CAN_OK) {
    Serial.print("Send failed: ");
    Serial.println(result);
  }

  delay(20);
}
}




