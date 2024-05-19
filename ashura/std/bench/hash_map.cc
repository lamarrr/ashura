

#include "ashura/std/hash_map.h"
#include "ashura/std/types.h"
#include "stdint.h"
#include <algorithm>
#include <benchmark/benchmark.h>
#include <map>
#include <unordered_map>

using namespace ash;

constexpr Tuple<Span<char const>, int> DATASET[] = {{"Lorem"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"dolor"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"adipiscing"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"Nullam"_span, 0},
                                                    {"ultricies"_span, 0},
                                                    {"purus"_span, 0},
                                                    {"facilisis"_span, 0},
                                                    {"orci"_span, 0},
                                                    {"euismod"_span, 0},
                                                    {"eleifend"_span, 0},
                                                    {"Pellentesque"_span, 0},
                                                    {"bibendum"_span, 0},
                                                    {"pretium"_span, 0},
                                                    {"quam"_span, 0},
                                                    {"et"_span, 0},
                                                    {"gravida"_span, 0},
                                                    {"Proin"_span, 0},
                                                    {"tortor"_span, 0},
                                                    {"urna"_span, 0},
                                                    {"convallis"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"neque"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"commodo"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"magna"_span, 0},
                                                    {"Praesent"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"nisl"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"purus"_span, 0},
                                                    {"pretium"_span, 0},
                                                    {"ultrices"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"Etiam"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"sapien"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"Morbi"_span, 0},
                                                    {"bibendum"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"dolor"_span, 0},
                                                    {"convallis"_span, 0},
                                                    {"egestas"_span, 0},
                                                    {"Interdum"_span, 0},
                                                    {"et"_span, 0},
                                                    {"malesuada"_span, 0},
                                                    {"fames"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"primis"_span, 0},
                                                    {"in"_span, 0},
                                                    {"faucibus"_span, 0},
                                                    {"Phasellus"_span, 0},
                                                    {"condimentum"_span, 0},
                                                    {"hendrerit"_span, 0},
                                                    {"tellus"_span, 0},
                                                    {"Nam"_span, 0},
                                                    {"eleifend"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"at"_span, 0},
                                                    {"ultrices"_span, 0},
                                                    {"sodales"_span, 0},
                                                    {"Aliquam"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"in"_span, 0},
                                                    {"ligula"_span, 0},
                                                    {"elementum"_span, 0},
                                                    {"dignissim"_span, 0},
                                                    {"Integer"_span, 0},
                                                    {"in"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"in"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"varius"_span, 0},
                                                    {"imperdiet"_span, 0},
                                                    {"a"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"dui"_span, 0},
                                                    {"Class"_span, 0},
                                                    {"aptent"_span, 0},
                                                    {"taciti"_span, 0},
                                                    {"sociosqu"_span, 0},
                                                    {"ad"_span, 0},
                                                    {"litora"_span, 0},
                                                    {"torquent"_span, 0},
                                                    {"per"_span, 0},
                                                    {"conubia"_span, 0},
                                                    {"nostra"_span, 0},
                                                    {"per"_span, 0},
                                                    {"inceptos"_span, 0},
                                                    {"himenaeos"_span, 0},
                                                    {"Nunc"_span, 0},
                                                    {"leo"_span, 0},
                                                    {"eros"_span, 0},
                                                    {"ullamcorper"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"fermentum"_span, 0},
                                                    {"id"_span, 0},
                                                    {"molestie"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"tellus"_span, 0},
                                                    {"Sed"_span, 0},
                                                    {"vel"_span, 0},
                                                    {"nunc"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"nulla"_span, 0},
                                                    {"varius"_span, 0},
                                                    {"porttitor"_span, 0},
                                                    {"Vivamus"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"molestie"_span, 0},
                                                    {"sapien"_span, 0},
                                                    {"Sed"_span, 0},
                                                    {"imperdiet"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"mauris"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"elementum"_span, 0},
                                                    {"metus"_span, 0},
                                                    {"tempor"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"Praesent"_span, 0},
                                                    {"et"_span, 0},
                                                    {"diam"_span, 0},
                                                    {"et"_span, 0},
                                                    {"orci"_span, 0},
                                                    {"blandit"_span, 0},
                                                    {"convallis"_span, 0},
                                                    {"rutrum"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"nisl"_span, 0},
                                                    {"Donec"_span, 0},
                                                    {"vulputate"_span, 0},
                                                    {"hendrerit"_span, 0},
                                                    {"nisi"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"rutrum"_span, 0},
                                                    {"Nullam"_span, 0},
                                                    {"faucibus"_span, 0},
                                                    {"tincidunt"_span, 0},
                                                    {"lectus"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"gravida"_span, 0},
                                                    {"est"_span, 0},
                                                    {"faucibus"_span, 0},
                                                    {"imperdiet"_span, 0},
                                                    {"Nam"_span, 0},
                                                    {"varius"_span, 0},
                                                    {"vehicula"_span, 0},
                                                    {"risus"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"tempus"_span, 0},
                                                    {"Aliquam"_span, 0},
                                                    {"erat"_span, 0},
                                                    {"volutpat"_span, 0},
                                                    {"In"_span, 0},
                                                    {"pellentesque"_span, 0},
                                                    {"auctor"_span, 0},
                                                    {"vulputate"_span, 0},
                                                    {"Suspendisse"_span, 0},
                                                    {"rhoncus"_span, 0},
                                                    {"magna"_span, 0},
                                                    {"quis"_span, 0},
                                                    {"tincidunt"_span, 0},
                                                    {"gravida"_span, 0},
                                                    {"libero"_span, 0},
                                                    {"ex"_span, 0},
                                                    {"egestas"_span, 0},
                                                    {"diam"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"hendrerit"_span, 0},
                                                    {"odio"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"velit"_span, 0},
                                                    {"Proin"_span, 0},
                                                    {"quis"_span, 0},
                                                    {"nulla"_span, 0},
                                                    {"placerat"_span, 0},
                                                    {"sagittis"_span, 0},
                                                    {"augue"_span, 0},
                                                    {"in"_span, 0},
                                                    {"mattis"_span, 0},
                                                    {"tortor"_span, 0},
                                                    {"Aliquam"_span, 0},
                                                    {"accumsan"_span, 0},
                                                    {"metus"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"nisl"_span, 0},
                                                    {"hendrerit"_span, 0},
                                                    {"non"_span, 0},
                                                    {"hendrerit"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"commodo"_span, 0},
                                                    {"Suspendisse"_span, 0},
                                                    {"bibendum"_span, 0},
                                                    {"euismod"_span, 0},
                                                    {"gravida"_span, 0},
                                                    {"Ut"_span, 0},
                                                    {"nisi"_span, 0},
                                                    {"libero"_span, 0},
                                                    {"facilisis"_span, 0},
                                                    {"nec"_span, 0},
                                                    {"erat"_span, 0},
                                                    {"a"_span, 0},
                                                    {"tempus"_span, 0},
                                                    {"ullamcorper"_span, 0},
                                                    {"risus"_span, 0},
                                                    {"Curabitur"_span, 0},
                                                    {"tortor"_span, 0},
                                                    {"mi"_span, 0},
                                                    {"suscipit"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"odio"_span, 0},
                                                    {"quis"_span, 0},
                                                    {"egestas"_span, 0},
                                                    {"suscipit"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"Nam"_span, 0},
                                                    {"id"_span, 0},
                                                    {"velit"_span, 0},
                                                    {"vel"_span, 0},
                                                    {"eros"_span, 0},
                                                    {"pharetra"_span, 0},
                                                    {"tristique"_span, 0},
                                                    {"nec"_span, 0},
                                                    {"at"_span, 0},
                                                    {"orci"_span, 0},
                                                    {"Lorem"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"dolor"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"adipiscing"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"Nulla"_span, 0},
                                                    {"dignissim"_span, 0},
                                                    {"magna"_span, 0},
                                                    {"quis"_span, 0},
                                                    {"venenatis"_span, 0},
                                                    {"ornare"_span, 0},
                                                    {"libero"_span, 0},
                                                    {"turpis"_span, 0},
                                                    {"sagittis"_span, 0},
                                                    {"tortor"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"posuere"_span, 0},
                                                    {"dui"_span, 0},
                                                    {"nunc"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"erat"_span, 0},
                                                    {"Aenean"_span, 0},
                                                    {"ullamcorper"_span, 0},
                                                    {"interdum"_span, 0},
                                                    {"mi"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"auctor"_span, 0},
                                                    {"Pellentesque"_span, 0},
                                                    {"tempor"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"urna"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"imperdiet"_span, 0},
                                                    {"orci"_span, 0},
                                                    {"iaculis"_span, 0},
                                                    {"In"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"fringilla"_span, 0},
                                                    {"augue"_span, 0},
                                                    {"Curabitur"_span, 0},
                                                    {"maximus"_span, 0},
                                                    {"nunc"_span, 0},
                                                    {"orci"_span, 0},
                                                    {"a"_span, 0},
                                                    {"elementum"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"blandit"_span, 0},
                                                    {"in"_span, 0},
                                                    {"Aliquam"_span, 0},
                                                    {"erat"_span, 0},
                                                    {"volutpat"_span, 0},
                                                    {"Donec"_span, 0},
                                                    {"feugiat"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"non"_span, 0},
                                                    {"scelerisque"_span, 0},
                                                    {"euismod"_span, 0},
                                                    {"Suspendisse"_span, 0},
                                                    {"sem"_span, 0},
                                                    {"diam"_span, 0},
                                                    {"consequat"_span, 0},
                                                    {"at"_span, 0},
                                                    {"tempus"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"lacinia"_span, 0},
                                                    {"et"_span, 0},
                                                    {"lectus"_span, 0},
                                                    {"Quisque"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"urna"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"vestibulum"_span, 0},
                                                    {"nisl"_span, 0},
                                                    {"at"_span, 0},
                                                    {"venenatis"_span, 0},
                                                    {"dui"_span, 0},
                                                    {"Donec"_span, 0},
                                                    {"maximus"_span, 0},
                                                    {"aliquam"_span, 0},
                                                    {"ornare"_span, 0},
                                                    {"Pellentesque"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"orci"_span, 0},
                                                    {"non"_span, 0},
                                                    {"ligula"_span, 0},
                                                    {"hendrerit"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"Vestibulum"_span, 0},
                                                    {"varius"_span, 0},
                                                    {"eros"_span, 0},
                                                    {"odio"_span, 0},
                                                    {"consequat"_span, 0},
                                                    {"aliquam"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"interdum"_span, 0},
                                                    {"condimentum"_span, 0},
                                                    {"Curabitur"_span, 0},
                                                    {"blandit"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"et"_span, 0},
                                                    {"varius"_span, 0},
                                                    {"Pellentesque"_span, 0},
                                                    {"lobortis"_span, 0},
                                                    {"nibh"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"consequat"_span, 0},
                                                    {"feugiat"_span, 0},
                                                    {"Phasellus"_span, 0},
                                                    {"ultricies"_span, 0},
                                                    {"enim"_span, 0},
                                                    {"purus"_span, 0},
                                                    {"et"_span, 0},
                                                    {"ultricies"_span, 0},
                                                    {"nulla"_span, 0},
                                                    {"molestie"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"Pellentesque"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"urna"_span, 0},
                                                    {"tristique"_span, 0},
                                                    {"a"_span, 0},
                                                    {"urna"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"fringilla"_span, 0},
                                                    {"consequat"_span, 0},
                                                    {"orci"_span, 0},
                                                    {"Nulla"_span, 0},
                                                    {"pharetra"_span, 0},
                                                    {"commodo"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"vel"_span, 0},
                                                    {"volutpat"_span, 0},
                                                    {"tellus"_span, 0},
                                                    {"porttitor"_span, 0},
                                                    {"a"_span, 0},
                                                    {"Praesent"_span, 0},
                                                    {"id"_span, 0},
                                                    {"augue"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"Mauris"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"aliquet"_span, 0},
                                                    {"dui"_span, 0},
                                                    {"Vestibulum"_span, 0},
                                                    {"fringilla"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"tincidunt"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"convallis"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"Aenean"_span, 0},
                                                    {"vel"_span, 0},
                                                    {"lacinia"_span, 0},
                                                    {"mauris"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"fringilla"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"Nulla"_span, 0},
                                                    {"pulvinar"_span, 0},
                                                    {"dolor"_span, 0},
                                                    {"quis"_span, 0},
                                                    {"dui"_span, 0},
                                                    {"aliquet"_span, 0},
                                                    {"elementum"_span, 0},
                                                    {"Interdum"_span, 0},
                                                    {"et"_span, 0},
                                                    {"malesuada"_span, 0},
                                                    {"fames"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"primis"_span, 0},
                                                    {"in"_span, 0},
                                                    {"faucibus"_span, 0},
                                                    {"Curabitur"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"ex"_span, 0},
                                                    {"libero"_span, 0},
                                                    {"In"_span, 0},
                                                    {"hac"_span, 0},
                                                    {"habitasse"_span, 0},
                                                    {"platea"_span, 0},
                                                    {"dictumst"_span, 0},
                                                    {"Donec"_span, 0},
                                                    {"tincidunt"_span, 0},
                                                    {"bibendum"_span, 0},
                                                    {"urna"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"suscipit"_span, 0},
                                                    {"Quisque"_span, 0},
                                                    {"at"_span, 0},
                                                    {"purus"_span, 0},
                                                    {"nec"_span, 0},
                                                    {"tellus"_span, 0},
                                                    {"sagittis"_span, 0},
                                                    {"fringilla"_span, 0},
                                                    {"In"_span, 0},
                                                    {"hac"_span, 0},
                                                    {"habitasse"_span, 0},
                                                    {"platea"_span, 0},
                                                    {"dictumst"_span, 0},
                                                    {"Curabitur"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"et"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"dapibus"_span, 0},
                                                    {"sagittis"_span, 0},
                                                    {"In"_span, 0},
                                                    {"tempor"_span, 0},
                                                    {"nisi"_span, 0},
                                                    {"in"_span, 0},
                                                    {"cursus"_span, 0},
                                                    {"tincidunt"_span, 0},
                                                    {"nisl"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"accumsan"_span, 0},
                                                    {"libero"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"imperdiet"_span, 0},
                                                    {"arcu"_span, 0},
                                                    {"massa"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"nisl"_span, 0},
                                                    {"In"_span, 0},
                                                    {"et"_span, 0},
                                                    {"vulputate"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"ullamcorper"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"Fusce"_span, 0},
                                                    {"quis"_span, 0},
                                                    {"augue"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"nunc"_span, 0},
                                                    {"feugiat"_span, 0},
                                                    {"ultrices"_span, 0},
                                                    {"Nulla"_span, 0},
                                                    {"est"_span, 0},
                                                    {"augue"_span, 0},
                                                    {"pretium"_span, 0},
                                                    {"et"_span, 0},
                                                    {"volutpat"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"mattis"_span, 0},
                                                    {"in"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"Aenean"_span, 0},
                                                    {"molestie"_span, 0},
                                                    {"magna"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"Ut"_span, 0},
                                                    {"lorem"_span, 0},
                                                    {"sapien"_span, 0},
                                                    {"placerat"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"porta"_span, 0},
                                                    {"et"_span, 0},
                                                    {"congue"_span, 0},
                                                    {"blandit"_span, 0},
                                                    {"neque"_span, 0},
                                                    {"Mauris"_span, 0},
                                                    {"tristique"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"a"_span, 0},
                                                    {"ullamcorper"_span, 0},
                                                    {"dignissim"_span, 0},
                                                    {"sapien"_span, 0},
                                                    {"nisi"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"diam"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"tincidunt"_span, 0},
                                                    {"nisl"_span, 0},
                                                    {"est"_span, 0},
                                                    {"id"_span, 0},
                                                    {"enim"_span, 0},
                                                    {"Quisque"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"nulla"_span, 0},
                                                    {"velit"_span, 0},
                                                    {"Vestibulum"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"libero"_span, 0},
                                                    {"turpis"_span, 0},
                                                    {"Phasellus"_span, 0},
                                                    {"tristique"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"non"_span, 0},
                                                    {"semper"_span, 0},
                                                    {"laoreet"_span, 0},
                                                    {"Maecenas"_span, 0},
                                                    {"vehicula"_span, 0},
                                                    {"congue"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"ultrices"_span, 0},
                                                    {"Sed"_span, 0},
                                                    {"ex"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"scelerisque"_span, 0},
                                                    {"non"_span, 0},
                                                    {"ligula"_span, 0},
                                                    {"et"_span, 0},
                                                    {"vestibulum"_span, 0},
                                                    {"tristique"_span, 0},
                                                    {"est"_span, 0},
                                                    {"In"_span, 0},
                                                    {"egestas"_span, 0},
                                                    {"porttitor"_span, 0},
                                                    {"tortor"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"tempor"_span, 0},
                                                    {"enim"_span, 0},
                                                    {"dictum"_span, 0},
                                                    {"non"_span, 0},
                                                    {"Vestibulum"_span, 0},
                                                    {"tincidunt"_span, 0},
                                                    {"leo"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"consequat"_span, 0},
                                                    {"pulvinar"_span, 0},
                                                    {"Morbi"_span, 0},
                                                    {"dictum"_span, 0},
                                                    {"mi"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"bibendum"_span, 0},
                                                    {"ullamcorper"_span, 0},
                                                    {"Integer"_span, 0},
                                                    {"nunc"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"varius"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"libero"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"viverra"_span, 0},
                                                    {"commodo"_span, 0},
                                                    {"ligula"_span, 0},
                                                    {"Pellentesque"_span, 0},
                                                    {"egestas"_span, 0},
                                                    {"scelerisque"_span, 0},
                                                    {"orci"_span, 0},
                                                    {"id"_span, 0},
                                                    {"interdum"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"tristique"_span, 0},
                                                    {"et"_span, 0},
                                                    {"Fusce"_span, 0},
                                                    {"non"_span, 0},
                                                    {"leo"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"Sed"_span, 0},
                                                    {"enim"_span, 0},
                                                    {"dui"_span, 0},
                                                    {"malesuada"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"eros"_span, 0},
                                                    {"non"_span, 0},
                                                    {"tristique"_span, 0},
                                                    {"dictum"_span, 0},
                                                    {"metus"_span, 0},
                                                    {"In"_span, 0},
                                                    {"non"_span, 0},
                                                    {"lectus"_span, 0},
                                                    {"feugiat"_span, 0},
                                                    {"pulvinar"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"interdum"_span, 0},
                                                    {"ligula"_span, 0},
                                                    {"Quisque"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"accumsan"_span, 0},
                                                    {"aliquet"_span, 0},
                                                    {"risus"_span, 0},
                                                    {"id"_span, 0},
                                                    {"dictum"_span, 0},
                                                    {"nibh"_span, 0},
                                                    {"Orci"_span, 0},
                                                    {"varius"_span, 0},
                                                    {"natoque"_span, 0},
                                                    {"penatibus"_span, 0},
                                                    {"et"_span, 0},
                                                    {"magnis"_span, 0},
                                                    {"dis"_span, 0},
                                                    {"parturient"_span, 0},
                                                    {"montes"_span, 0},
                                                    {"nascetur"_span, 0},
                                                    {"ridiculus"_span, 0},
                                                    {"mus"_span, 0},
                                                    {"Vestibulum"_span, 0},
                                                    {"sodales"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"non"_span, 0},
                                                    {"luctus"_span, 0},
                                                    {"egestas"_span, 0},
                                                    {"Ut"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"neque"_span, 0},
                                                    {"Nulla"_span, 0},
                                                    {"nec"_span, 0},
                                                    {"arcu"_span, 0},
                                                    {"interdum"_span, 0},
                                                    {"auctor"_span, 0},
                                                    {"nisi"_span, 0},
                                                    {"non"_span, 0},
                                                    {"posuere"_span, 0},
                                                    {"dui"_span, 0},
                                                    {"Aliquam"_span, 0},
                                                    {"sodales"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"diam"_span, 0},
                                                    {"gravida"_span, 0},
                                                    {"porta"_span, 0},
                                                    {"Donec"_span, 0},
                                                    {"iaculis"_span, 0},
                                                    {"massa"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"nulla"_span, 0},
                                                    {"ultrices"_span, 0},
                                                    {"id"_span, 0},
                                                    {"pretium"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"sodales"_span, 0},
                                                    {"Curabitur"_span, 0},
                                                    {"dignissim"_span, 0},
                                                    {"purus"_span, 0},
                                                    {"ex"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"finibus"_span, 0},
                                                    {"nisi"_span, 0},
                                                    {"volutpat"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"Suspendisse"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"nibh"_span, 0},
                                                    {"non"_span, 0},
                                                    {"odio"_span, 0},
                                                    {"varius"_span, 0},
                                                    {"porttitor"_span, 0},
                                                    {"Nulla"_span, 0},
                                                    {"elementum"_span, 0},
                                                    {"ullamcorper"_span, 0},
                                                    {"ultrices"_span, 0},
                                                    {"Sed"_span, 0},
                                                    {"mattis"_span, 0},
                                                    {"purus"_span, 0},
                                                    {"libero"_span, 0},
                                                    {"non"_span, 0},
                                                    {"condimentum"_span, 0},
                                                    {"leo"_span, 0},
                                                    {"dapibus"_span, 0},
                                                    {"tristique"_span, 0},
                                                    {"In"_span, 0},
                                                    {"malesuada"_span, 0},
                                                    {"eleifend"_span, 0},
                                                    {"tortor"_span, 0},
                                                    {"non"_span, 0},
                                                    {"ornare"_span, 0},
                                                    {"enim"_span, 0},
                                                    {"fringilla"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"Sed"_span, 0},
                                                    {"dolor"_span, 0},
                                                    {"leo"_span, 0},
                                                    {"commodo"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"felis"_span, 0},
                                                    {"viverra"_span, 0},
                                                    {"dapibus"_span, 0},
                                                    {"convallis"_span, 0},
                                                    {"tellus"_span, 0},
                                                    {"Phasellus"_span, 0},
                                                    {"rutrum"_span, 0},
                                                    {"volutpat"_span, 0},
                                                    {"leo"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"imperdiet"_span, 0},
                                                    {"neque"_span, 0},
                                                    {"Cras"_span, 0},
                                                    {"libero"_span, 0},
                                                    {"orci"_span, 0},
                                                    {"feugiat"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"enim"_span, 0},
                                                    {"a"_span, 0},
                                                    {"rutrum"_span, 0},
                                                    {"egestas"_span, 0},
                                                    {"felis"_span, 0},
                                                    {"Curabitur"_span, 0},
                                                    {"vel"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"eros"_span, 0},
                                                    {"vestibulum"_span, 0},
                                                    {"tempor"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"quis"_span, 0},
                                                    {"purus"_span, 0},
                                                    {"Aliquam"_span, 0},
                                                    {"condimentum"_span, 0},
                                                    {"rhoncus"_span, 0},
                                                    {"facilisis"_span, 0},
                                                    {"Nulla"_span, 0},
                                                    {"euismod"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"lorem"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"laoreet"_span, 0},
                                                    {"orci"_span, 0},
                                                    {"auctor"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"Cras"_span, 0},
                                                    {"gravida"_span, 0},
                                                    {"risus"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"mauris"_span, 0},
                                                    {"scelerisque"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"pretium"_span, 0},
                                                    {"ante"_span, 0},
                                                    {"malesuada"_span, 0},
                                                    {"Nulla"_span, 0},
                                                    {"facilisis"_span, 0},
                                                    {"molestie"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"pellentesque"_span, 0},
                                                    {"Morbi"_span, 0},
                                                    {"id"_span, 0},
                                                    {"odio"_span, 0},
                                                    {"nunc"_span, 0},
                                                    {"Cras"_span, 0},
                                                    {"fermentum"_span, 0},
                                                    {"augue"_span, 0},
                                                    {"a"_span, 0},
                                                    {"felis"_span, 0},
                                                    {"aliquet"_span, 0},
                                                    {"placerat"_span, 0},
                                                    {"Mauris"_span, 0},
                                                    {"lorem"_span, 0},
                                                    {"ex"_span, 0},
                                                    {"fermentum"_span, 0},
                                                    {"vel"_span, 0},
                                                    {"euismod"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"fermentum"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"nulla"_span, 0},
                                                    {"Mauris"_span, 0},
                                                    {"quis"_span, 0},
                                                    {"pulvinar"_span, 0},
                                                    {"lorem"_span, 0},
                                                    {"in"_span, 0},
                                                    {"dapibus"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"Nunc"_span, 0},
                                                    {"et"_span, 0},
                                                    {"venenatis"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"tincidunt"_span, 0},
                                                    {"rhoncus"_span, 0},
                                                    {"sem"_span, 0},
                                                    {"Lorem"_span, 0},
                                                    {"ipsum"_span, 0},
                                                    {"dolor"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"adipiscing"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"Aenean"_span, 0},
                                                    {"dictum"_span, 0},
                                                    {"volutpat"_span, 0},
                                                    {"leo"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"commodo"_span, 0},
                                                    {"nisl"_span, 0},
                                                    {"eleifend"_span, 0},
                                                    {"eleifend"_span, 0},
                                                    {"Donec"_span, 0},
                                                    {"id"_span, 0},
                                                    {"elit"_span, 0},
                                                    {"viverra"_span, 0},
                                                    {"bibendum"_span, 0},
                                                    {"mi"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"tincidunt"_span, 0},
                                                    {"dui"_span, 0},
                                                    {"Mauris"_span, 0},
                                                    {"vel"_span, 0},
                                                    {"lacinia"_span, 0},
                                                    {"est"_span, 0},
                                                    {"Mauris"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"scelerisque"_span, 0},
                                                    {"diam"_span, 0},
                                                    {"non"_span, 0},
                                                    {"fringilla"_span, 0},
                                                    {"nisi"_span, 0},
                                                    {"Cras"_span, 0},
                                                    {"iaculis"_span, 0},
                                                    {"neque"_span, 0},
                                                    {"leo"_span, 0},
                                                    {"pretium"_span, 0},
                                                    {"pellentesque"_span, 0},
                                                    {"est"_span, 0},
                                                    {"eleifend"_span, 0},
                                                    {"a"_span, 0},
                                                    {"Integer"_span, 0},
                                                    {"egestas"_span, 0},
                                                    {"rutrum"_span, 0},
                                                    {"semper"_span, 0},
                                                    {"Mauris"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"iaculis"_span, 0},
                                                    {"diam"_span, 0},
                                                    {"placerat"_span, 0},
                                                    {"luctus"_span, 0},
                                                    {"arcu"_span, 0},
                                                    {"Praesent"_span, 0},
                                                    {"pellentesque"_span, 0},
                                                    {"egestas"_span, 0},
                                                    {"massa"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"varius"_span, 0},
                                                    {"Aliquam"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"leo"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"turpis"_span, 0},
                                                    {"lobortis"_span, 0},
                                                    {"pretium"_span, 0},
                                                    {"id"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"tellus"_span, 0},
                                                    {"Phasellus"_span, 0},
                                                    {"ut"_span, 0},
                                                    {"turpis"_span, 0},
                                                    {"ac"_span, 0},
                                                    {"libero"_span, 0},
                                                    {"pretium"_span, 0},
                                                    {"eleifend"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"tortor"_span, 0},
                                                    {"Curabitur"_span, 0},
                                                    {"dictum"_span, 0},
                                                    {"id"_span, 0},
                                                    {"nulla"_span, 0},
                                                    {"non"_span, 0},
                                                    {"vehicula"_span, 0},
                                                    {"Quisque"_span, 0},
                                                    {"nec"_span, 0},
                                                    {"purus"_span, 0},
                                                    {"vulputate"_span, 0},
                                                    {"felis"_span, 0},
                                                    {"pellentesque"_span, 0},
                                                    {"lacinia"_span, 0},
                                                    {"Donec"_span, 0},
                                                    {"vel"_span, 0},
                                                    {"consectetur"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"Aenean"_span, 0},
                                                    {"lectus"_span, 0},
                                                    {"sapien"_span, 0},
                                                    {"tincidunt"_span, 0},
                                                    {"a"_span, 0},
                                                    {"tortor"_span, 0},
                                                    {"sed"_span, 0},
                                                    {"fermentum"_span, 0},
                                                    {"tristique"_span, 0},
                                                    {"nibh"_span, 0},
                                                    {"Ut"_span, 0},
                                                    {"eget"_span, 0},
                                                    {"justo"_span, 0},
                                                    {"lorem"_span, 0},
                                                    {"Morbi"_span, 0},
                                                    {"efficitur"_span, 0},
                                                    {"elementum"_span, 0},
                                                    {"efficitur"_span, 0},
                                                    {"Vestibulum"_span, 0},
                                                    {"auctor"_span, 0},
                                                    {"sem"_span, 0},
                                                    {"vel"_span, 0},
                                                    {"efficitur"_span, 0},
                                                    {"auctor"_span, 0},
                                                    {"Sed"_span, 0},
                                                    {"diam"_span, 0},
                                                    {"nisi"_span, 0},
                                                    {"dignissim"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"lectus"_span, 0},
                                                    {"sit"_span, 0},
                                                    {"amet"_span, 0},
                                                    {"lacinia"_span, 0},
                                                    {"pulvinar"_span, 0},
                                                    {"nisi"_span, 0},
                                                    {"Pellentesque"_span, 0},
                                                    {"malesuada"_span, 0},
                                                    {"dolor"_span, 0},
                                                    {"vitae"_span, 0},
                                                    {"egestas"_span, 0},
                                                    {"tempus"_span, 0},
                                                    {"Mauris"_span, 0},
                                                    {"placerat"_span, 0},
                                                    {"ex"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"est"_span, 0},
                                                    {"posuere"_span, 0},
                                                    {"dapibus"_span, 0},
                                                    {"Pellentesque"_span, 0},
                                                    {"bibendum"_span, 0},
                                                    {"dui"_span, 0},
                                                    {"nec"_span, 0},
                                                    {"sodales"_span, 0},
                                                    {"congue"_span, 0},
                                                    {"lacus"_span, 0},
                                                    {"nibh"_span, 0},
                                                    {"pulvinar"_span, 0},
                                                    {"diam"_span, 0},
                                                    {"eu"_span, 0},
                                                    {"tempus"_span, 0},
                                                    {"sapien"_span, 0},
                                                    {"eros"_span, 0},
                                                    {"quis"_span, 0},
                                                    {"arcu"_span, 0},
                                                    {"Class"_span, 0},
                                                    {"aptent"_span, 0},
                                                    {"taciti"_span, 0},
                                                    {"sociosqu"_span, 0},
                                                    {"ad"_span, 0}};

static void BM_HashMap16(benchmark::State &state)
{
  StrHashMap<int> map;
  int             count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      bool exists;
      (void) map.insert(exists, nullptr, dp.v0, dp.v1);
    }
    for (auto &dp : DATASET)
    {
      if (map.has(dp.v0))
      {
        count++;
      }
    }
    for (auto &dp : DATASET)
    {
      map.erase(dp.v0);
    }
  }
  printf("num unique inserts: %d\n", count);
}

static void BM_HashMap8(benchmark::State &state)
{
  StrHashMap<int, u8> map;
  int                 count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      bool exists;
      (void) map.insert(exists, nullptr, dp.v0, dp.v1);
    }
    for (auto &dp : DATASET)
    {
      if (map.has(dp.v0))
      {
        count++;
      }
    }
    for (auto &dp : DATASET)
    {
      map.erase(dp.v0);
    }
  }
  printf("num unique inserts: %d\n", count);
}

static void BM_HashMap32(benchmark::State &state)
{
  StrHashMap<int, u32> map;
  int                  count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      bool exists;
      (void) map.insert(exists, nullptr, dp.v0, dp.v1);
    }
    for (auto &dp : DATASET)
    {
      if (map.has(dp.v0))
      {
        count++;
      }
    }
    for (auto &dp : DATASET)
    {
      map.erase(dp.v0);
    }
  }
  printf("num unique inserts: %d\n", count);
}

static void BM_HashMap64(benchmark::State &state)
{
  StrHashMap<int, u64> map;
  int                  count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      bool exists;
      (void) map.insert(exists, nullptr, dp.v0, dp.v1);
    }
    for (auto &dp : DATASET)
    {
      if (map.has(dp.v0))
      {
        count++;
      }
    }
    for (auto &dp : DATASET)
    {
      map.erase(dp.v0);
    }
  }
  printf("num unique inserts: %d\n", count);
}

template <class T>
struct std_allocator
{
  using size_type       = size_t;
  using difference_type = ptrdiff_t;
  using pointer         = T *;
  using const_pointer   = const T *;
  using reference       = T &;
  using const_reference = const T &;
  using value_type      = T;

  template <class U>
  struct rebind
  {
    typedef std_allocator<U> other;
  };
  std_allocator() throw()
  {
  }
  std_allocator(const std_allocator &) throw()
  {
  }

  template <class U>
  std_allocator(const std_allocator<U> &) throw()
  {
  }

  ~std_allocator() throw()
  {
  }

  pointer address(reference x) const
  {
    return &x;
  }
  const_pointer address(const_reference x) const
  {
    return &x;
  }

  pointer alloc(size_type s, void const * = 0)
  {
    pointer temp;
    if (!default_allocator.t_alloc(s, &temp))
    {
      throw std::bad_alloc();
    }
    return temp;
  }

  void dealloc(pointer p, size_type s)
  {
    default_allocator.t_dealloc(p, s);
  }

  size_type max_size() const throw()
  {
    return std::numeric_limits<size_t>::max() / sizeof(T);
  }

  void construct(pointer p, const T &val)
  {
    new ((void *) p) T(val);
  }

  void destroy(pointer p)
  {
    p->~T();
  }
};

static void BM_StdHashMap(benchmark::State &state)
{
  std::unordered_map<Span<char const>, int, StrHasher, StrEqual,
                     std_allocator<std::pair<Span<char const> const, int>>>
      map;
  int count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      map.emplace(dp.v0, dp.v1);
    }
    for (auto &dp : DATASET)
    {
      if (map.contains(dp.v0))
      {
        count++;
      }
    }
    for (auto &dp : DATASET)
    {
      map.erase(dp.v0);
    }
  }
  printf("num unique inserts: %d\n", count);
}

template <>
struct std::less<Span<char const>>
{
  bool operator()(Span<char const> a, Span<char const> b) const
  {
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
  }
};

template <>
struct std::hash<Span<char const>>
{
  std::hash<std::string_view> hash;
  size_t                      operator()(Span<char const> str) const
  {
    return hash(std::string_view{str.data(), str.size()});
  }
};

static void BM_StdHashMapDefaultHash(benchmark::State &state)
{
  std::unordered_map<Span<char const>, int, std::hash<Span<char const>>,
                     StrEqual,
                     std_allocator<std::pair<Span<char const> const, int>>>
      map;
  int count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      map.emplace(dp.v0, dp.v1);
    }
    for (auto &dp : DATASET)
    {
      if (map.contains(dp.v0))
      {
        count++;
      }
    }
    for (auto &dp : DATASET)
    {
      map.erase(dp.v0);
    }
  }
  printf("num unique inserts: %d\n", count);
}

static void BM_StdHashMapDefaultHashDefaultAlloc(benchmark::State &state)
{
  std::unordered_map<Span<char const>, int, std::hash<Span<char const>>,
                     StrEqual>
      map;
  int count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      map.emplace(dp.v0, dp.v1);
    }
    for (auto &dp : DATASET)
    {
      if (map.contains(dp.v0))
      {
        count++;
      }
    }
    for (auto &dp : DATASET)
    {
      map.erase(dp.v0);
    }
  }
  printf("num unique inserts: %d\n", count);
}

static void BM_StdOrderedMapDefaultAlloc(benchmark::State &state)
{
  std::map<Span<char const>, int> map;
  int                             count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      map.emplace(dp.v0, dp.v1);
    }
    for (auto &dp : DATASET)
    {
      if (map.contains(dp.v0))
      {
        count++;
      }
    }
    for (auto &dp : DATASET)
    {
      map.erase(dp.v0);
    }
  }
  printf("num unique inserts: %d\n", count);
}

constexpr int ITERATIONS = 100'000;
BENCHMARK(BM_HashMap8)->Iterations(ITERATIONS);
BENCHMARK(BM_HashMap16)->Iterations(ITERATIONS);
BENCHMARK(BM_HashMap32)->Iterations(ITERATIONS);
BENCHMARK(BM_HashMap64)->Iterations(ITERATIONS);
BENCHMARK(BM_StdHashMap)->Iterations(ITERATIONS);
BENCHMARK(BM_StdHashMapDefaultHash)->Iterations(ITERATIONS);
BENCHMARK(BM_StdHashMapDefaultHashDefaultAlloc)->Iterations(ITERATIONS);
BENCHMARK(BM_StdOrderedMapDefaultAlloc)->Iterations(ITERATIONS);
BENCHMARK_MAIN();