SYSTEM_MODE(SEMI_AUTOMATIC); // dont start wifi initially

// Define constants
const unsigned int loopsPerReading = 1000; //  1000 loops (approx 250 per backpane, around 125 of which will be low)
const unsigned int bpLow = 500; // threshold at which we assume the backplane is pulled low
const unsigned int stableReadingCount = 20; // 20 consecutive readings seems to be about when the scales decide the weight is stable

// Define variables
bool pD1, pD2, pD3, pD4, pD5, pD6, pD7; // digital pin inputs
unsigned int pA1, pA2, pA3, pA4; // analogue pin inputs from the backplanes
unsigned int B1n = 0, B2n = 0, B3n = 0, B4n = 0; // count of measurements from each backplane
unsigned int B1c[8] = {0}, B2c[8] = {0}, B3c[8] = {0}, B4c[8] = {0}, Breset[8] = {0};  // count of when measurements were high from each digital input for each backplane
bool B1[8] = {0}, B2[8] = {0}, B3[8] = {0}, B4[8] = {0}; // result of averaging digits over a number of loops
int reading = 0, readingPrev = 0; // reading and previous readings
unsigned int readingMatchCount = 0; // count of how many times the reading has consecutively matched
char publishString[40] = {0}, printString[40] = {0};
unsigned int count = 0; // number of loops since last reading
unsigned long lastInputTime = millis(); //
unsigned int tOut = 10000; // allow 10 seconds before turning off


int getDigit(int i, int j, int errVal) {
    if (!B1[i] && !B1[j] && !B2[i] && !B2[j] && !B3[i] && !B3[j] && !B4[i]) return 0; // treat blank as zero
    if ( B1[i] &&  B1[j] &&  B2[i] && !B2[j] &&  B3[i] &&  B3[j] &&  B4[i]) return 0;
    if (!B1[i] && !B1[j] &&  B2[i] && !B2[j] &&  B3[i] && !B3[j] && !B4[i]) return 1;
    if ( B1[i] &&  B1[j] && !B2[i] &&  B2[j] &&  B3[i] && !B3[j] &&  B4[i]) return 2;
    if ( B1[i] && !B1[j] &&  B2[i] &&  B2[j] &&  B3[i] && !B3[j] &&  B4[i]) return 3;
    if (!B1[i] && !B1[j] &&  B2[i] &&  B2[j] &&  B3[i] &&  B3[j] && !B4[i]) return 4;
    if ( B1[i] && !B1[j] &&  B2[i] &&  B2[j] && !B3[i] &&  B3[j] &&  B4[i]) return 5;
    if ( B1[i] &&  B1[j] &&  B2[i] &&  B2[j] && !B3[i] &&  B3[j] &&  B4[i]) return 6;
    if (!B1[i] && !B1[j] &&  B2[i] && !B2[j] &&  B3[i] && !B3[j] &&  B4[i]) return 7;
    if ( B1[i] &&  B1[j] &&  B2[i] &&  B2[j] &&  B3[i] &&  B3[j] &&  B4[i]) return 8;
    if ( B1[i] && !B1[j] &&  B2[i] &&  B2[j] &&  B3[i] &&  B3[j] &&  B4[i]) return 9;
    return errVal; 
}

void setup() {
    //pinMode(D0, INPUT_PULLDOWN);
    pinMode(D1, INPUT_PULLDOWN);
    pinMode(D2, INPUT_PULLDOWN);
    pinMode(D3, INPUT_PULLDOWN);
    pinMode(D4, INPUT_PULLDOWN);
    pinMode(D5, INPUT_PULLDOWN);
    pinMode(D6, INPUT_PULLDOWN);
    pinMode(D7, INPUT_PULLDOWN);
    //pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A4, INPUT); 
    pinMode(A5, INPUT_PULLDOWN); // connect wifi button
    Serial.begin(9600);
    Serial.println("Spark core scales, initial setup");
}  


void loop() {
    pA1 = analogRead(A1);
    pA2 = analogRead(A2);
    pA3 = analogRead(A3);
    pA4 = analogRead(A4);
    pD1 = digitalRead(D1);
    pD2 = digitalRead(D2);
    pD3 = digitalRead(D3);
    pD4 = digitalRead(D4);
    pD5 = digitalRead(D5);
    pD6 = digitalRead(D6);
    pD7 = digitalRead(D7);
    
    if (pA1<bpLow) { // read from a backplane if it is low
        B1c[1] += pD1;
        B1c[2] += pD2;
        B1c[3] += pD3;
        B1c[4] += pD4;
        B1c[5] += pD5;
        B1c[6] += pD6;
        B1c[7] += pD7;
        B1n++;
    }
    if (pA2<bpLow) {
        B2c[1] += pD1;
        B2c[2] += pD2;
        B2c[3] += pD3;
        B2c[4] += pD4;
        B2c[5] += pD5;
        B2c[6] += pD6;
        B2c[7] += pD7;
        B2n++;
    }
    if (pA3<bpLow) {
        B3c[1] += pD1;
        B3c[2] += pD2;
        B3c[3] += pD3;
        B3c[4] += pD4;
        B3c[5] += pD5;
        B3c[6] += pD6;
        B3c[7] += pD7;
        B3n++;
    }
    if (pA4<bpLow) {
        B4c[1] += pD1;
        B4c[2] += pD2;
        B4c[3] += pD3;
        B4c[4] += pD4;
        B4c[5] += pD5;
        B4c[6] += pD6;
        B4c[7] += pD7;
        B4n++;
    }
    
    count++;
    
    if (count>loopsPerReading-1) { // read out the average 
        for (int i=1; i<8; i++) { // average value of each digit in each backplane
            B1[i] = (1.0*B1c[i] / B1n >0.5);
            B2[i] = (1.0*B2c[i] / B2n >0.5);
            B3[i] = (1.0*B3c[i] / B3n >0.5);
            B4[i] = (1.0*B4c[i] / B4n >0.5);
        }
        
        reading =  1000*getDigit(1, 0, -10000) + 100*getDigit(3, 2, -10000) + 10*getDigit(5, 4, -10000); // multiply by an extra 10 so that we are working with integers
        if (reading>0) { // ignore if the tens or units are wrong
            reading += 1*getDigit(7, 6, 5); // assume an average of 0.5 if these are wrong
            sprintf(printString,"Weight: %d.%d kg", reading/10, abs(reading)%10);
            Serial.println(printString);
            if (reading==readingPrev) {
                readingMatchCount++;
            } else {
                readingPrev = reading;
                readingMatchCount = 0;
            }
            lastInputTime = millis();
        }
        
        if (readingMatchCount==stableReadingCount) { // weight is deemed stable
            sprintf(printString,"Weight to be logged: %d.%d kg", reading/10, abs(reading)%10);
            Serial.println(printString);
            sprintf(publishString,"%d.%d", reading/10, abs(reading)%10);
            Serial.println("Connecting to wifi...");
            Spark.connect();
        }
 
        count = 0; // reset counters
        B1n = 0; B2n = 0; B3n = 0; B4n = 0;
        B1c[0] = 0; B1c[1] = 0; B1c[2] = 0; B1c[3] = 0; B1c[4] = 0; B1c[5] = 0; B1c[6] = 0; B1c[7] = 0;
        B2c[0] = 0; B2c[1] = 0; B2c[2] = 0; B2c[3] = 0; B2c[4] = 0; B2c[5] = 0; B2c[6] = 0; B2c[7] = 0;
        B3c[0] = 0; B3c[1] = 0; B3c[2] = 0; B3c[3] = 0; B3c[4] = 0; B3c[5] = 0; B3c[6] = 0; B3c[7] = 0;
        B4c[0] = 0; B4c[1] = 0; B4c[2] = 0; B4c[3] = 0; B4c[4] = 0; B4c[5] = 0; B4c[6] = 0; B4c[7] = 0;
    }
    
    if (Spark.connected() & publishString[0]!=0) {
        lastInputTime = millis();
        Serial.println("Connected to wifi, publishing weight");
        Spark.publish("scalesLog",publishString,60,PRIVATE);
        publishString[0] = 0;
        readingMatchCount = 0;
        delay(12000);
        Serial.println("Sleeping after publishing...");
        Spark.sleep(A0,RISING);        
    }
    
    if (digitalRead(A5)==1) {
        Serial.println("Button pressed to connect to wifi...");
        Spark.connect();
        lastInputTime = millis();
        tOut = 100000; // increase timeout
    }
    
    if (millis()-lastInputTime>tOut) {
        Serial.println("Sleeping due to lack of input...");
        Spark.sleep(A0,RISING);        
    }
}
