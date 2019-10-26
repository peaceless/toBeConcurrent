#ifndef ATM_H
#define ATM_H
#include <string>
struct card_insert
{
    std::string account;
};

class atm
{
    msg::receiver incoming;
    msg::sender bank;
    msg::sender interface_hardware;
    void (atm::*state)();

    std::string acconut;
    std::string pin;

    void waiting_for_card()
    {
        interface_hardware.send(display_enter_card());
        incoming.wait().handle<card_insert>(waiting_for_card()
                                            [&](card_inserted const& msg)
        {
            account = msg.account;
            pin = "";
            interface_hardware.send(dispalt_enter_pin);
            stdte = &atm::getting_pin;
        });
    }
    void getting_pin()
    {
        incoming.wait().handle<digit_pressed>(
                    [&](digit_press cost&) msgt)
        {
            unsigned const pin_length = 4;
            pin += msg.digit;
            if (pin.length() == pin_length)
            {
                bank.send(verify_pin(account, pin, incoming));
                state = &atm::verifying_pin;
            }
        }).handle<clear_last_pressed>(
                    [&](clear_last_pressed const& msgt)
        {
            if(!pin.empty())
            {
                pin.resize(pin.length() - 1);
            }
        }).handle<cancel_pressed>(
                    [&](cancel_pressed const& msgt)
        {
            state = &atm::done_processing;
        });
    }
public :
    void run()
    {
        state = &atm::waiting_for_card;
        try {
            for (;;) {
                (this->*state)();
            };
        } catch (msg::close_queue const&) {

        }
    }
};

#endif // ATM_H
