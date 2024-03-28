#ifndef TMCDRIVER_h
#define TMCDRIVER_h

#define R_SENSE 0.075f
#define EN_PIN           22 // Enable
#define STEP_PIN         4 // Step
#define DIR_PIN          15 // Dir
#define CS_PIN           16 // Chip select

#include <TMCStepper.h>
#include "FastAccelStepper.h"

struct TMC5160Controller {
    TMC5160Stepper &driver;
    FastAccelStepperEngine &engine;
    FastAccelStepper *stepper = NULL;
    bool enabled = false;
    bool direction = false;
    bool running = false;
    uint16_t stepsPerRotation = 200;
    uint16_t microsteps = 256;
    uint16_t current = 30;
    uint32_t maxSpeed = 400;
    uint32_t maxAccel = 200;
    const char* msteps;
    const char* pwmfr;
    const char* freewh;
    const int EN;
    const int DIR;
    const int STEP;

    TMC5160Controller(TMC5160Stepper &driver, FastAccelStepperEngine &engine, const int STEP, const int DIR, const int EN) : driver {driver}, engine {engine}, STEP {STEP}, DIR {DIR}, EN {EN} {}

    void init() {
        pinMode(EN, OUTPUT);
        pinMode(STEP, OUTPUT);
        digitalWrite(EN, LOW);      // Enable driver in hardware
        driver.begin();                 //  SPI: Init CS pins and possible SW SPI pins
        if (driver.version() == 0xFF || driver.version() == 0) Serial.println("Driver communication error");
        Serial.print("Driver firmware version: ");
        Serial.println(driver.version());
        if (driver.sd_mode()) Serial.println("Driver is hardware configured for Step & Dir mode");
        if (driver.drv_enn()) Serial.println("Driver is not hardware enabled");


        Serial.print("DRV_STATUS=0b");
        Serial.println(driver.DRV_STATUS(), BIN);
        initDriver();

        engine.init();
        stepper = engine.stepperConnectToPin(STEP);
        if (stepper) {
            stepper->setDirectionPin(DIR);
            stepper->setSpeedInHz(200*microsteps);       // 200 steps/s
            stepper->setAcceleration(40*microsteps);    // 40 steps/s²
        }
        else Serial.println("Stepper ERROR");
    }

    void initDriver() {
        driver.GSTAT(1);
        driver.defaults();
        // driver.toff(3);
        // enable();
        driver.rms_current(500);
        driver.en_pwm_mode(true);
        driver.s2g_level(9);
        driver.s2vs_level(9);
        driver.bbmclks(2);
        driver.shaft(true);
        disable();
        loadSettings();
        // stepper->attachToPulseCounter(6, -200*microsteps, 200*microsteps);
    }

    void loadSettings() {
        // String driverName = "drv" + String(STEP);
        // preferences.begin(driverName.c_str(), false);
        // driver.shaft(preferences.getBool("shaft", false));
        // driver.en_pwm_mode(preferences.getBool("stchp", true));
        // driver.GLOBAL_SCALER(preferences.getInt("scaler", 128));
        // driver.ihold(preferences.getInt("ihold", 8));
        // driver.irun(preferences.getInt("irun", 16));
        // driver.s2vs_level(preferences.getInt("s2vs_level", 9));
        // driver.s2g_level(preferences.getInt("s2g_level", 9));
        // driver.bbmclks(preferences.getInt("bbmclks", 3));
        // stepsPerRotation = preferences.getInt("stpr", stepsPerRotation);
        // if (stepper) {
        //     Serial.println(preferences.getInt("maxspd", stepper->getSpeedInMilliHz()/1000));
        //     stepper->setSpeedInHz(preferences.getInt("maxspd", stepper->getSpeedInMilliHz()/1000));
        //     stepper->setAcceleration(preferences.getInt("maxaccel", stepper->getAcceleration()));
        // }
        // preferences.end();
    }

    void saveSettings() {
        // String driverName = "drv" + String(STEP);
        // if (driver.version() == 0xFF || driver.version() == 0) Serial.println("Driver communication error");
        // else {
        //     preferences.begin(driverName.c_str(), false);
        //     preferences.putBool("shaft", driver.shaft());
        //     preferences.putBool("stchp", driver.en_pwm_mode());
        //     preferences.putInt("scaler", driver.GLOBAL_SCALER());
        //     preferences.putInt("ihold", driver.ihold());
        //     preferences.putInt("irun", driver.irun());
        //     preferences.putInt("s2vs_level", driver.s2vs_level());
        //     preferences.putInt("s2g_level", driver.s2g_level());
        //     preferences.putInt("bbmclks", driver.bbmclks());
        // }
        // preferences.putInt("stpr", stepsPerRotation);
        // preferences.putInt("maxspd", stepper->getSpeedInMilliHz()/1000);
        // Serial.println(stepper->getSpeedInMilliHz()/1000);
        // preferences.putInt("maxaccel", stepper->getAcceleration());
        // preferences.end();
    }

    void setMaxSpeed(uint32_t sp) {
        stepper->setSpeedInHz(sp*microsteps);
    }

    uint32_t getMaxSpeed() {
        return stepper->getSpeedInMilliHz()/(1000*microsteps);
    }

    void setSpeed(double sp) {
        sp = min(max(-1., sp), 1.);
        Serial.print("Set : ");
        Serial.println(sp);
        if (sp != 0) setMaxSpeed(abs(sp*maxSpeed));
        if (sp == 0) stepper->stopMove();
        else if (sp < 0) stepper->runBackward();
        else stepper->runForward();
    }

    double getSpeed() {
        return double(stepper->getCurrentSpeedInMilliHz())/double(1000*microsteps*maxSpeed);
    }

    void stop() {
        setSpeed(0);
    }

    int32_t move() {
        return (stepper->targetPos()-stepper->getCurrentPosition())/microsteps;
    }

    void moveR(int32_t move) {
        stepper->moveTo(stepper->getCurrentPosition() + move * microsteps);
    }

    void moveR(double angle) {
        stepper->moveTo(stepper->getCurrentPosition() + angle*stepsPerRotation/360. * microsteps);
        // Serial.println(driver.XACTUAL());
        // driver.RAMPMODE(0);
        // driver.VSTART(100);
        // driver.AMAX(stepper->getAcceleration());
        // driver.a1(stepper->getAcceleration());
        // driver.VMAX(stepper->getMaxSpeedInHz());
        // driver.v1(stepper->getMaxSpeedInHz());
        // driver.DMAX(stepper->getMaxSpeedInHz());
        // driver.d1(stepper->getMaxSpeedInHz());
        // driver.VSTOP(100);
        // driver.XTARGET(driver.XACTUAL() + angle*stepsPerRotation/360. * microsteps);
    }

    void move(double angle) {
        stepper->move((angle*stepsPerRotation/360.-double(this->move()))*microsteps);
    }

    void move(int32_t move) {
        stepper->move((move-this->move())*microsteps);
    }

    float mod(float a, float N) {return a - N*floor(a/N);}

    void setAngle(double angle) {
        double dist = angle - getAngle();
        moveR(dist);
    }

    double getAngle() {
        return mod(stepper->getCurrentPosition()*360./double(microsteps), 360.);
    }

    double getTargetAngle() {
        return mod(stepper->targetPos()*360./double(microsteps), 360.);
    }

    void enable() {
        // digitalWrite(ST_EN, LOW);
        driver.toff(3);
        enabled = true;
    }

    bool isEnabled() {
        return driver.isEnabled();
    }

    void disable() {
        // digitalWrite(ST_EN, HIGH);
        driver.toff(0);
        enabled = false;
    }

    void setMicroSteps(uint16_t ms) {
        microsteps = ms;
        driver.microsteps(ms);
    }

    const char* getMicroSteps() {
        uint16_t m = driver.microsteps();
        microsteps = m ? m : 1;
        if (msteps != NULL) free((char*)msteps);
        msteps = strdup(String(microsteps).c_str());
        return msteps;
    }

    void setAcceleration(double acc) {
        stepper->setAcceleration(acc*maxAccel*microsteps);
    }

    double getAcceleration() {
        return double(stepper->getAcceleration())/double(microsteps*maxAccel);
    }

    uint32_t getStepsToStop() {
        return getSpeed() * getSpeed() / (getAcceleration() * 2);
    }

    const char* pwm_freq() {
        if (pwmfr != NULL) free((char*)pwmfr);
        pwmfr = strdup(String(driver.pwm_freq()).c_str());
        return pwmfr;
    }

    const char* getFreeWheel() {
        if (freewh != NULL) free((char*)freewh);
        freewh = strdup(String(driver.freewheel()).c_str());
        return freewh;
    }
};

#endif