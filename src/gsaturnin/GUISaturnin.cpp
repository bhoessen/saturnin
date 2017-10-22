#include "GUISaturnin.h"

#include "saturnin/Assert.h"
#include "saturnin/Solver.h"

#include "imgui/imgui.h"

#ifdef WIN32
#pragma warning( disable : 4244 )
#endif /* WIN32 */

gsaturnin::GUISaturnin::GUISaturnin(int argc, char ** argv) : launcher(argc, argv), propagations("Propagations"), nbClauses("Nb Clauses"), 
conflicts("Conflicts"), assignLevel("Assign Level"), memory("Usage (Mb)")
#ifdef PROFILE
,profile_propagateMono("Monowatch propagation"),
profile_propagateBin("Binary propagation"),
profile_propagateWatched("Watched propagation"),
profile_reduce("Reduce"),
profile_analyze("Analyze"),
profile_simplify("Simplify")
#endif /* PROFILE */
{
}                                                                                   
                                                                                    
void gsaturnin::GUISaturnin::start() {
    launcher.solve();
}

void gsaturnin::GUISaturnin::stop() {
    ASSERT(launcher.getSolver() != nullptr);
    launcher.stop();
}


float clause_repartition(void* data, int idx) {
    auto* solver = static_cast<saturnin::Solver*>(data);
#ifndef PROFILE
    return static_cast<float>(solver->getAllocator().getPool(idx).getSize());
#else
    if (idx == 2) return static_cast<float>(solver->getNbBinaryClauses());
    return static_cast<float>(solver->getAllocator().getClauseRepartition().get(idx));
#endif /* PROFILE */
}

float lbd_repartition(void* data, int idx) {
    auto* solver = static_cast<saturnin::Solver*>(data);
    return static_cast<float>(solver->getLBDDistribution()[static_cast<unsigned int>(idx)]);
}

float memusage_clausePools(void* data, int idx) {
    auto* solver = static_cast<saturnin::Solver*>(data);
    return solver->getAllocator().getPool(idx).getMemoryFootprint()/(1024.f);
}

void gsaturnin::GUISaturnin::draw() {
    updateData();
    ImGui::SetNextWindowSize(ImVec2(442, 235), ImGuiSetCond_Always);
    ImGui::SetNextWindowPos(ImVec2(12, 17), ImGuiSetCond_Always);
    ImGui::Begin("Instance");
    ImGui::Text("File: %s", launcher.getInstanceName());
    ImGui::Text("Nb variables:      %12u", launcher.getNbVar());
    ImGui::Text("Nb clause:         %12u", launcher.getNbClauses());
    ImGui::Text("Avg clause lenght: %12u", launcher.getAvgClauseLength());
    ImGui::Text("Max clause lenght: %12u", launcher.getMaxClauseLength());
    ImGui::Text("Time:              %12.2f", launcher.getEllapsedTime());


#ifndef SATURNIN_PARALLEL
    auto* solver = launcher.getSolver();

    ImGui::Text("Solution:         %13s", solver->getState() == wUnknown ? "Unknown" : (solver->getState() == wTrue ? "SATISFIABLE" : "UNSATISFIABLE"));
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(442, 249), ImGuiSetCond_Always);
    ImGui::SetNextWindowPos(ImVec2(12, 269), ImGuiSetCond_Always);
    ImGui::Begin("Solver");
    propagations.draw();
    nbClauses.draw();
    conflicts.draw();
    assignLevel.draw();
#ifdef PROFILE
    ImGui::PlotHistogram("Clause repartition", clause_repartition, static_cast<void*>(const_cast<saturnin::Solver*>(solver)), solver->getAllocator().getClauseRepartition().getSize());
#else
    ImGui::PlotHistogram("Clause repartition", clause_repartition, static_cast<void*>(const_cast<saturnin::Solver*>(solver)), solver->getAllocator().getNbPools());
#endif
    ImGui::PlotHistogram("LBD distribution", lbd_repartition, static_cast<void*>(const_cast<saturnin::Solver*>(solver)), solver->getLBDDistribution().getSize());
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(442, 235), ImGuiSetCond_Always);
    ImGui::SetNextWindowPos(ImVec2(471, 17), ImGuiSetCond_Always);
    ImGui::Begin("Memory");
    memory.draw();
    ImGui::PlotHistogram("Clauses (Kb)", memusage_clausePools, static_cast<void*>(const_cast<saturnin::Solver*>(solver)), solver->getAllocator().getNbPools());
    ImGui::End();

#ifdef PROFILE
    ImGui::SetNextWindowSize(ImVec2(442, 249), ImGuiSetCond_Always);
    ImGui::SetNextWindowPos(ImVec2(471, 269), ImGuiSetCond_Always);
    ImGui::Begin("Profile");
    profile_propagateMono.draw();
    profile_propagateBin.draw();
    profile_propagateWatched.draw();
    profile_reduce.draw();
    profile_analyze.draw();
    //profile_simplify.draw();
    ImGui::End();
#endif /* PROFILE */


    ImGui::SetNextWindowSize(ImVec2(442, 249*2), ImGuiSetCond_Always);
    ImGui::SetNextWindowPos(ImVec2(471+442+17, 17), ImGuiSetCond_Always);
    ImGui::Begin("Variables");

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    static const float sz = 4.0f;
    static ImVec4 colTrue = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    static ImVec4 colFalse = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    static ImVec4 colUnknown = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    static const ImU32 colors[] = { ImColor(colTrue), ImColor(colFalse), ImColor(colUnknown) };
    const unsigned int nbPerRow = 65U;
    static int j = 0;
    static const unsigned int maxNBVarVisible = nbPerRow * 70;
    char buf[256];
    snprintf(buf, 256, "page %d/%d", j+1, (solver->getNbVar() / maxNBVarVisible)+1);
    ImGui::DragInt("Select offset", &j, 1, 0, solver->getNbVar() / maxNBVarVisible, buf);
    {
        const ImVec2 p = ImGui::GetCursorScreenPos();
        float x = p.x + 4.0f, y = p.y + 4.0f, spacing = 2.0f;
        float thickness = 1.0f;
        draw_list->_Path.reserve(draw_list->_Path.size() + 4 * solver->getNbVar());
        for (unsigned int n = j*maxNBVarVisible; n < solver->getNbVar() && n < (j+1)*maxNBVarVisible; n++)
        {
            unsigned int nn = n % maxNBVarVisible;
            float xp = x + (sz + spacing)*(nn % nbPerRow);
            float yp = y + (sz + spacing)*(nn / nbPerRow);
            draw_list->AddRect(ImVec2(xp, yp), ImVec2(xp + sz, yp + sz), colors[static_cast<int>(solver->getVarValue(n))], 0.0f, ~0, thickness); 
        }
    }

    ImGui::End();

#else
    ImGui::End();
#endif


}

void gsaturnin::GUISaturnin::updateData() {
    auto* solver = launcher.getSolver();
    auto ellapsed = launcher.getEllapsedTime();
    if (solver->getState() == wUnknown) {
        propagations.push(static_cast<float>(solver->getNbPropagation()), ellapsed);
        nbClauses.push(static_cast<float>(solver->getNbLearntClauses()));
        conflicts.push(static_cast<float>(solver->getNbConflict()), ellapsed);
        assignLevel.push(static_cast<float>(solver->getAssignationLevel()));
        memory.push(static_cast<float>(solver->getMemoryFootprint() / (1024.0*1024.0)));
#ifdef PROFILE
        auto duration = solver->__profile_propagateMono
                      + solver->__profile_propagateBin 
                      + solver->__profile_propagateWatched 
                      + solver->__profile_reduce 
                      + solver->__profile_analyze;
        //duration = 100;
        profile_propagateMono.push(static_cast<float>(solver->__profile_propagateMono*100.0 / duration));
        profile_propagateBin.push(static_cast<float>(solver->__profile_propagateBin*100.0 / duration));
        profile_propagateWatched.push(static_cast<float>(solver->__profile_propagateWatched*100.0 / duration));
        profile_reduce.push(static_cast<float>(solver->__profile_reduce*100.0 / duration));
        profile_analyze.push(static_cast<float>(solver->__profile_analyze*100.0 / duration));
        profile_simplify.push(static_cast<float>(solver->__profile_simplify*100.0 / duration));
#endif
    }
}
