// SPDX-License-Identifier: AGPL-3.0-or-later
#include "reports.h"
#include "subghz.h"
namespace rf {
bool valid(unsigned khz) { return khz >= 433000 && khz <= 435000; }
namespace {
unsigned fixtureKhz, sequence;
State demoOpen() {
    sequence = 0;
    return State::Active;
}
bool demoTune(unsigned khz) {
    fixtureKhz = khz;
    return valid(khz);
}
bool demoRead(int &dbm) {
    const int distance = int(fixtureKhz) - 433900;
    const int attenuation = (distance < 0 ? -distance : distance) / 8;
    dbm = -48 - (attenuation < 46 ? attenuation : 46) - int((sequence++ * 7) % 11);
    return true;
}
void demoClose() {}
} // namespace
const Backend simulated{demoOpen, demoTune, demoRead, demoClose};
unsigned Session::frequency() const { return sweep ? center - 8 * step + bin * step : center; }
void Session::stop(Result reason) {
    if (backend) backend->close();
    backend = nullptr;
    result = reason;
}
void Session::begin(const Backend &driver, bool scan, unsigned khz, unsigned spacing, unsigned now) {
    stop();
    *this = Session{};
    sweep = scan;
    center = khz;
    step = spacing;
    if (!valid(khz) || (step != 50 && step != 100) ||
        (scan && (!valid(khz - 8 * step) || !valid(khz + 8 * step)))) {
        result = Result::Fault;
        return;
    }
    const State state = driver.open(); // Manager rolls back a failed open.
    if (state != State::Active) {
        result = state == State::Conflict      ? Result::Conflict
                 : state == State::Unavailable ? Result::Missing
                                               : Result::Fault;
        return;
    }
    backend = &driver;
    result = Result::Running;
    startMs = tunedMs = now;
    if (!backend->tune(frequency())) stop(Result::Fault);
}
void Session::tick(unsigned now) {
    if (!backend) return;
    if (now - startMs >= 30000) {
        stop(Result::Timeout);
        return;
    }
    if (now - tunedMs < 100) return;
    int value;
    if (!backend->read(value)) {
        stop(Result::Fault);
        return;
    }
    if (!sweep && count && value >= -70 && at(count - 1).dbm >= -70) activityMs += now - at(count - 1).ms;
    if (backend == &simulated) ws::recorder.observe(ws::SimRssi, ws::Origin::Simulation, value);
    history[head] = {now, value};
    head = (head + 1) % capacity;
    if (count < capacity) ++count;
    current[bin] = value;
    if (bin >= seen || value > hold[bin]) hold[bin] = value;
    if (bin >= seen) seen = bin + 1;
    if (value > peak) {
        peak = value;
        peakKhz = frequency();
    }
    tunedMs = now;
    if (sweep) {
        bin = (bin + 1) % bins;
        if (!backend->tune(frequency())) stop(Result::Fault);
    }
}
} // namespace rf
