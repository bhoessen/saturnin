#pragma once
#ifndef SATURNIN_VALUEEVOLUTION_H
#define SATURNIN_VALUEEVOLUTION_H

#include "imgui/imgui.h"

namespace gsaturnin {

    /**
     * The purpose of this class is to show an evolution of a value over the last N iterations.
     * It is also possible to automatically compute the derivative if the corresponding template parameter is set to true
     */
    template <int N, bool derivative = false>
    class ValueEvolution {
    public:
        ValueEvolution(const char* aValueName) : valueName(aValueName), nbFilled(0), next(0) {
            for (auto &v : values) {
                v = 0.f;
            }
        }

        void push(float val) {
            avg = avg - values[next] + val;
            values[next] = val;
            next = (next + 1) % N;
            nbFilled = nbFilled < N ? nbFilled + 1 : N;
            lastValue = val;
        }

        void push(float val, float ellapsedTime) {
            static_assert(derivative, "The function push(float val, float ellapsedTime) should only be used when derivative is activated");
            push((val - lastValue) / ellapsedTime);
            lastValue = val;
        }

        float operator[](int index) const {
            if (nbFilled < N) {
                return values[index];
            }
            else {
                return values[(next + index) % N];
            }
        }

        void draw() const {
            ImGui::Text("%-20s: %12.2f (avg: %12.2f)", valueName, lastValue, nbFilled > 0 ? avg/nbFilled : 0.f);
            ImGui::PlotLines("", ValueEvolution<N,derivative>::value_getter, static_cast<void*>(const_cast<ValueEvolution*>(this)), nbFilled);
        }

    private:

        static float value_getter(void* data, int idx) {
            const auto* valEvo = static_cast<ValueEvolution<N>*>(data);
            return valEvo->operator[](idx);
        }

        const char* valueName;
        float values[N];
        float lastValue = 0.f;
        float avg = 0;
        int nbFilled;
        int next;

    };

}

#endif
