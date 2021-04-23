void setupJoystick() {
    Joystick.setXAxisRange(minX, maxX);
    Joystick.setYAxisRange(minY, maxY);
    Joystick.begin();
}

void updateJoystickPos() {
    Joystick.setXAxis(pos[0]);
    Joystick.setYAxis(pos[1]);
}

void updateEffects(bool recalculate){
    for (int i =0; i < 2; i++) {
        effects[i].frictionMaxPositionChange = 25; // TODO: find proper values for these automatically
        effects[i].inertiaMaxAcceleration = 10;
        effects[i].damperMaxVelocity = 150;
    }

    effects[0].springMaxPosition = maxX;
    effects[1].springMaxPosition = maxY;
    effects[0].springPosition = pos[0];
    effects[1].springPosition = pos[1];

    unsigned long currentMillis;
    currentMillis = millis();
    int16_t diffTime = currentMillis - lastEffectsUpdate;

    if (diffTime > 0 && recalculate) {
        lastEffectsUpdate = currentMillis;
        int16_t positionChangeX = pos[0] - lastX;
        int16_t positionChangeY = pos[1] - lastY;
        int16_t velX = positionChangeX / diffTime;
        int16_t velY = positionChangeY / diffTime;
        int16_t accelX = ((velX - lastVelX) * 10) / diffTime;
        int16_t accelY = ((velY - lastVelY) * 10) / diffTime;
    
        effects[0].frictionPositionChange = velX;
        effects[1].frictionPositionChange = velY;
        effects[0].inertiaAcceleration = accelX;
        effects[1].inertiaAcceleration = accelY;
        effects[0].damperVelocity = velX;
        effects[1].damperVelocity = velY;

        #ifdef DEBUG
        write_order(LOG);
        Serial.print(currentMillis);
        Serial.print("X");
        Serial.print(pos[0]);
        Serial.print("Y");
        Serial.print(pos[1]);
        Serial.print("C");
        Serial.print(positionChangeX);
        Serial.print(",");
        Serial.print(positionChangeY);
        Serial.print("V");
        Serial.print(velX);
        Serial.print(",");
        Serial.print(velY);
        Serial.print("A");
        Serial.print(accelX);
        Serial.print(",");
        Serial.print(accelY);
        #endif

        lastX = pos[0];
        lastY = pos[1];
        lastVelX = velX;
        lastVelY = velY;
        lastAccelX = accelX;
        lastAccelY = accelY;
    } else {
        effects[0].frictionPositionChange = lastVelX;
        effects[1].frictionPositionChange = lastVelY;
        effects[0].inertiaAcceleration = lastAccelX;
        effects[1].inertiaAcceleration = lastAccelY;
        effects[0].damperVelocity = lastVelX;
        effects[1].damperVelocity = lastVelY;
    }

    Joystick.setEffectParams(effects);
    Joystick.getForce(forces);

    #ifdef DEBUG
    if (diffTime > 0 && recalculate) {
        Serial.print("F");
        Serial.print(forces[0]);
        Serial.print(",");
        Serial.println(forces[1]);
    }
    #endif
}
