#include "Controller.h"

constexpr uint32_t s_button_mask =
    (1 << BUTTON_RIGHT) | (1 << BUTTON_DOWN) |
    (1 << BUTTON_LEFT) | (1 << BUTTON_UP) |
    (1 << BUTTON_SEL);

// Make sure someone has called Controller::begin() before running functions that
// rely on this.
#define checkIsBegun()                                                                      \
    if (this->hasBegun == false)                                                            \
    {                                                                                       \
        s_printf("Cannot run command %s because begin has not been called.", __FUNCTION__); \
        return false;                                                                       \
    }

#define semTake(c) c->takeSemaphore();
#define semGive(c) c->giveSemaphore();

ButtonState getPressedButtons(uint32_t mask)
{
    ButtonState bs;

    if (!(mask & BUTTON_RIGHT))
    {
        bs.a = ButtonPressState::Down;
    }

    if (!(mask & BUTTON_LEFT))
    {
        bs.x = ButtonPressState::Down;
    }

    if (!(mask & BUTTON_DOWN))
    {
        bs.b = ButtonPressState::Down;
    }

    if (!(mask & BUTTON_UP))
    {
        bs.y = ButtonPressState::Down;
    }

    if (!(mask & BUTTON_SEL))
    {
        bs.sel = ButtonPressState::Up;
    }

    d_printf("PRESS: x=%d y=%d a=%d b=%d s=%d bin=" BYTE_TO_BINARY_PATTERN "\n",
             (int)bs.x, (int)bs.y,
             (int)bs.a, (int)bs.b,
             (int)bs.sel,
             BYTE_TO_BINARY(mask));

    return bs;
}

Controller::Controller(uint8_t irqPin, uint8_t addr)
{
    this->irqPin = irqPin;
    this->addr = addr;
    this->sem = xSemaphoreCreateBinary();
    this->buttonQueue = xQueueCreate(10, sizeof(Controller));
    this->giveSemaphore();
}

Controller::~Controller()
{
    detachInterrupt(this->irqPin);
}

bool Controller::begin(controller_button_callback buttonCallback, controller_analog_stick_move_callback stickCallback)
{
    if (buttonCallback == nullptr ||
        stickCallback == nullptr)
    {
        s_println(F("ERROR: Missing button or stick callback parameter value."));
        return false;
    }

    this->buttonCallback = buttonCallback;
    this->stickCallback = stickCallback;

    if (ss.begin(addr) == false)
    {
        s_println(F("ERROR: Could not start Seesaw"));
        return false;
    }

    this->ss.pinModeBulk(s_button_mask, INPUT_PULLUP);
    this->ss.setGPIOInterrupts(s_button_mask, 1);
    this->hasBegun = true;
    BaseType_t result = xTaskCreate(
        Controller::buttonPressTask,
        "buttonPressTask",
        10000,
        this,
        1,
        NULL);

    if (result != pdPASS)
    {
        s_printf("ERROR: Could not create button press task. Result: %d\n", result);
        this->hasBegun = false;
        return false;
    }

    result = xTaskCreate(
        Controller::stickMoveTask,
        "analogStickTask",
        10000,
        this,
        1,
        NULL);

    if (result != pdPASS)
    {
        s_printf("ERROR: Could not create analog stick task. Result: %d\n", result);
        this->hasBegun = false;
        return false;
    }

    pinMode(this->irqPin, INPUT);
    attachInterrupt(this->irqPin, this->isr, FALLING);

    return true;
}

bool Controller::calibrate(uint16_t upCorrection,
                           uint16_t downCorrection,
                           uint16_t leftCorrection,
                           uint16_t rightCorrection,
                           uint16_t minStickMovementH,
                           uint16_t minStickMovementV)
{
    checkIsBegun();

    s_println(F("Calibrating analog stick"));

    // Hold on to the semaphore for the entirety of the function to make sure
    // we aren't reading the stick while calibration is happening and messing up
    // positional calcluations
    semTake(this);

    this->upCorrection = upCorrection;
    this->downCorrection = -downCorrection;
    this->leftCorrection = -leftCorrection;
    this->rightCorrection = rightCorrection;

    this->minStickMovementH = minStickMovementH;
    this->minStickMovementV = minStickMovementV;

    this->x_ctr = ss.analogRead(JOYSTICK_H);
    this->y_ctr = ss.analogRead(JOYSTICK_V);
    s_printf("Calibrated analog stick. Center X=%d Y=%d. Correction values: L=%d R=%d U=%d D=%d\n",
             this->x_ctr,
             this->y_ctr,
             this->leftCorrection,
             this->rightCorrection,
             this->upCorrection,
             this->downCorrection);
    this->hasRecalibrated = true;
    semGive(this);
    return true;
}

bool Controller::end()
{
    this->hasBegun = false;
    return true;
}

void IRAM_ATTR Controller::onButtonPress()
{
    xQueueSendFromISR(buttonQueue, this, NULL);
}

void Controller::buttonPressTask(void *p)
{
    s_println("buttonPressTask() enter");
    uint32_t lastValue = 0;
    Controller *c = static_cast<Controller *>(p);
    while (c->hasBegun == true)
    {
        if (xQueueReceive(c->buttonQueue, &c, 10000000) == false)
        {
            continue;
        }

        semTake(c);
        uint32_t v = c->ss.digitalReadBulk(s_button_mask);
        semGive(c);

        // Simple debounce. If the value is identical to before we will just discard it.
        if (lastValue != v)
        {
            lastValue = v;
            c->buttonCallback(getPressedButtons(v));
        }
    }

    s_println("buttonPressTask() exit");
    vTaskDelete(NULL);
}

void Controller::stickMoveTask(void *p)
{
    int16_t x = -1;
    int16_t y = -1;
    s_println(F("stickMoveTask() begin"));
    Controller *c = static_cast<Controller *>(p);
    while (c->hasBegun == true)
    {
        bool hasRecalibrated;
        semTake(c);
        int16_t new_x = c->ss.analogRead(JOYSTICK_H);
        int16_t new_y = c->ss.analogRead(JOYSTICK_V);
        hasRecalibrated = c->hasRecalibrated;
        c->hasRecalibrated = false;
        semGive(c);

        // Ignore minute changes to the analog stick movement as every reading will
        // be slightly different
        if (new_x <= x - c->minStickMovementH ||
            new_x >= x + c->minStickMovementH ||
            new_y <= y - c->minStickMovementV ||
            new_y >= y + c->minStickMovementV ||
            hasRecalibrated == true)
        {
            AnalogStickMovement as = c->getStickPosition(new_x, new_y);
            c->stickCallback(as);
            x = new_x;
            y = new_y;
        }

        delay(100); // Short enough to have quick reads, long enough not to saturate the i2c bus
    }

    s_println(F("stickMoveTask() end"));
    vTaskDelete(NULL);
}

bool Controller::giveSemaphore()
{
    return xSemaphoreGive(this->sem);
}

bool Controller::takeSemaphore()
{
    return xSemaphoreTake(this->sem, portMAX_DELAY);
}

AnalogStickMovement Controller::getStickPosition(int16_t &x, int16_t &y)
{
    AnalogStickMovement as;

    // Best effort to determine if we're centered or not
    as.isCentered = this->x_ctr >= max(0, x + this->leftCorrection) &&
                    this->x_ctr <= max(0, x + this->rightCorrection) &&
                    this->y_ctr <= max(0, y + this->upCorrection) &&
                    this->y_ctr >= max(0, y + downCorrection);

    // If we're out of bounds, recalculate
    x = x < 0 ? 0 : x > 1024 ? 1024
                             : x;
    y = y < 0 ? 0 : y > 1024 ? 1024
                             : y;

    // Convert the position to radian values
    double xr = x / 4 - 128;
    double yr = y / 4 - 128;

    // Get the angle and position position from center
    as.angle = -atan2(-xr, yr) * (180.0 / PI);
    as.velocity = sqrt(pow(xr, 2) + pow(yr, 2));
    return as;
}