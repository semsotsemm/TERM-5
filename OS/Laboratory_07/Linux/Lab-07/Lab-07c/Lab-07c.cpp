#include <iostream>
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstdint>

using namespace std;

int main()
{
    setlocale(LC_CTYPE, "");

    unsigned long long iteration = 0;
    int pass_seconds = 0;

    int waitable_timer = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    itimerspec first_activation = {};
    first_activation.it_value.tv_sec = 3;
    first_activation.it_interval.tv_sec = 3;
    timerfd_settime(waitable_timer, 0, &first_activation, nullptr);

    uint64_t expirations;
    while (true)
    {
        iteration++;
        if (read(waitable_timer, &expirations, sizeof(expirations)) > 0)
        {
            pass_seconds += 3;
            cout << "Количество итераций за " << pass_seconds << " секунд: " << iteration << '\n';
            if (pass_seconds == 15)
            {
                break;
            }
        }
    }

    close(waitable_timer);
    return 0;
}