#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct ExpeditionState {
    int oxygen;
    int fuel;
};

struct CrewMember {
    std::string name;
    int energy;
};

struct SupplyCache {
    std::string name;
    int quantity;
};

// FIX 1: state is now a reference, so the subtraction changes the
// caller's expedition instead of a local copy.
void travel(ExpeditionState& state, int oxygenUsed, int fuelUsed) {
    state.oxygen -= oxygenUsed;
    state.fuel -= fuelUsed;
}

// FIX 2: the loop variable is now a reference, so each pass changes the
// crew member stored in the vector instead of a temporary copy.
void restCrew(std::vector<CrewMember>& crew, int amount) {
    for (CrewMember& member : crew) {
        member.energy += amount;
    }
}

SupplyCache& findCache(std::vector<SupplyCache>& caches,
                       const std::string& name) {
    for (SupplyCache& cache : caches) {
        if (cache.name == name) {
            return cache;
        }
    }
    throw std::invalid_argument("Unknown supply cache: " + name);
}

// FIX 3: selected is now declared as a reference, so it refers to the
// cache that findCache found inside the vector.
void resupply(std::vector<SupplyCache>& caches, const std::string& name,
              int amount) {
    SupplyCache& selected = findCache(caches, name);
    selected.quantity += amount;
}

// The report only reads data, so every parameter is a const reference.
// This avoids copying the vectors and lets the compiler reject any
// accidental change to the expedition, crew, or caches.
void printReport(const ExpeditionState& state,
                 const std::vector<CrewMember>& crew,
                 const std::vector<SupplyCache>& caches) {
    std::cout << "EXPEDITION STATUS\n";
    std::cout << "Oxygen: " << state.oxygen << '\n';
    std::cout << "Fuel: " << state.fuel << "\n\nCREW\n";
    for (const CrewMember& member : crew) {
        std::cout << member.name << ": " << member.energy << " energy\n";
    }
    std::cout << "\nSUPPLY CACHES\n";
    for (const SupplyCache& cache : caches) {
        std::cout << cache.name << ": " << cache.quantity << '\n';
    }
}

int main() {
    ExpeditionState expedition{100, 60};
    std::vector<CrewMember> crew{{"Mira", 40}, {"Sol", 65}};
    std::vector<SupplyCache> caches{{"Water", 3}, {"Food", 2}};

    travel(expedition, 20, 15);
    restCrew(crew, 10);
    resupply(caches, "Food", 4);
    printReport(expedition, crew, caches);
}

/*
EXPLANATION OF THE COPYING PROBLEMS

Before the fixes, the starter printed Oxygen 100, Fuel 60, Mira 40,
Sol 65, Water 3, Food 2. Every update ran, but none of them stuck.

1. travel()
   What was copied: the whole ExpeditionState. The parameter was
   declared as ExpeditionState state, so main's expedition was copied
   into a new local object when travel was called.
   Why the original did not change: the function subtracted oxygen and
   fuel from that local copy, and the copy was destroyed when the
   function returned. main's expedition was never touched.
   How the fix changes that: the parameter is now ExpeditionState&
   state. A reference is another name for main's expedition, so
   state.oxygen -= oxygenUsed changes the original object.

2. restCrew()
   What was copied: each CrewMember, one at a time. The vector itself
   was already passed by reference, but the loop used
   for (auto member : crew), and plain auto makes member a new
   CrewMember copied from each element.
   Why the original did not change: member.energy += amount raised the
   energy of the copy, which was thrown away at the end of each pass.
   The objects stored in the vector kept their old energy.
   How the fix changes that: the loop now uses CrewMember& member, so
   member refers directly to each element in the vector and every
   stored crew member gains energy.

3. resupply()
   What was copied: the Food SupplyCache. findCache correctly returns a
   SupplyCache& that refers to the element inside the vector, but the
   line auto selected = findCache(caches, name) stored it in a plain
   auto variable. auto drops the reference, so selected was a new
   SupplyCache copied from the Food element.
   Why the original did not change: selected.quantity += amount added to
   the copy, and the Food object in the vector stayed at 2.
   How the fix changes that: selected is now declared as SupplyCache&,
   so it binds to the element findCache found. Only that stored cache
   changes, and Water is left alone because findCache never returns it
   when the name is Food.

Read-only function: printReport() also took all three arguments by
value. That did not cause a wrong answer, but it copied both vectors on
every call and would have allowed the report to change its own copies
without anyone noticing. It now takes const references, which avoids
the copies and makes any attempted change a compile error.
*/
