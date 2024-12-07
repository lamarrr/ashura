/// SPDX-License-Identifier: MIT
#include "ashura/std/map.h"
#include "ashura/std/types.h"
#include "stdint.h"
#include <algorithm>
#include <benchmark/benchmark.h>
#include <map>
#include <unordered_map>

using namespace ash;

constexpr Tuple<Span<char const>, int> DATASET[] = {{"Lorem"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"dolor"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"adipiscing"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"Nullam"_str, 0},
                                                    {"ultricies"_str, 0},
                                                    {"purus"_str, 0},
                                                    {"facilisis"_str, 0},
                                                    {"orci"_str, 0},
                                                    {"euismod"_str, 0},
                                                    {"eleifend"_str, 0},
                                                    {"Pellentesque"_str, 0},
                                                    {"bibendum"_str, 0},
                                                    {"pretium"_str, 0},
                                                    {"quam"_str, 0},
                                                    {"et"_str, 0},
                                                    {"gravida"_str, 0},
                                                    {"Proin"_str, 0},
                                                    {"tortor"_str, 0},
                                                    {"urna"_str, 0},
                                                    {"convallis"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"neque"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"commodo"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"magna"_str, 0},
                                                    {"Praesent"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"nisl"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"purus"_str, 0},
                                                    {"pretium"_str, 0},
                                                    {"ultrices"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"Etiam"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"sapien"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"Morbi"_str, 0},
                                                    {"bibendum"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"dolor"_str, 0},
                                                    {"convallis"_str, 0},
                                                    {"egestas"_str, 0},
                                                    {"Interdum"_str, 0},
                                                    {"et"_str, 0},
                                                    {"malesuada"_str, 0},
                                                    {"fames"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"primis"_str, 0},
                                                    {"in"_str, 0},
                                                    {"faucibus"_str, 0},
                                                    {"Phasellus"_str, 0},
                                                    {"condimentum"_str, 0},
                                                    {"hendrerit"_str, 0},
                                                    {"tellus"_str, 0},
                                                    {"Nam"_str, 0},
                                                    {"eleifend"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"at"_str, 0},
                                                    {"ultrices"_str, 0},
                                                    {"sodales"_str, 0},
                                                    {"Aliquam"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"in"_str, 0},
                                                    {"ligula"_str, 0},
                                                    {"elementum"_str, 0},
                                                    {"dignissim"_str, 0},
                                                    {"Integer"_str, 0},
                                                    {"in"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"in"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"varius"_str, 0},
                                                    {"imperdiet"_str, 0},
                                                    {"a"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"dui"_str, 0},
                                                    {"Class"_str, 0},
                                                    {"aptent"_str, 0},
                                                    {"taciti"_str, 0},
                                                    {"sociosqu"_str, 0},
                                                    {"ad"_str, 0},
                                                    {"litora"_str, 0},
                                                    {"torquent"_str, 0},
                                                    {"per"_str, 0},
                                                    {"conubia"_str, 0},
                                                    {"nostra"_str, 0},
                                                    {"per"_str, 0},
                                                    {"inceptos"_str, 0},
                                                    {"himenaeos"_str, 0},
                                                    {"Nunc"_str, 0},
                                                    {"leo"_str, 0},
                                                    {"eros"_str, 0},
                                                    {"ullamcorper"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"fermentum"_str, 0},
                                                    {"id"_str, 0},
                                                    {"molestie"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"tellus"_str, 0},
                                                    {"Sed"_str, 0},
                                                    {"vel"_str, 0},
                                                    {"nunc"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"nulla"_str, 0},
                                                    {"varius"_str, 0},
                                                    {"porttitor"_str, 0},
                                                    {"Vivamus"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"molestie"_str, 0},
                                                    {"sapien"_str, 0},
                                                    {"Sed"_str, 0},
                                                    {"imperdiet"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"mauris"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"elementum"_str, 0},
                                                    {"metus"_str, 0},
                                                    {"tempor"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"Praesent"_str, 0},
                                                    {"et"_str, 0},
                                                    {"diam"_str, 0},
                                                    {"et"_str, 0},
                                                    {"orci"_str, 0},
                                                    {"blandit"_str, 0},
                                                    {"convallis"_str, 0},
                                                    {"rutrum"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"nisl"_str, 0},
                                                    {"Donec"_str, 0},
                                                    {"vulputate"_str, 0},
                                                    {"hendrerit"_str, 0},
                                                    {"nisi"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"rutrum"_str, 0},
                                                    {"Nullam"_str, 0},
                                                    {"faucibus"_str, 0},
                                                    {"tincidunt"_str, 0},
                                                    {"lectus"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"gravida"_str, 0},
                                                    {"est"_str, 0},
                                                    {"faucibus"_str, 0},
                                                    {"imperdiet"_str, 0},
                                                    {"Nam"_str, 0},
                                                    {"varius"_str, 0},
                                                    {"vehicula"_str, 0},
                                                    {"risus"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"tempus"_str, 0},
                                                    {"Aliquam"_str, 0},
                                                    {"erat"_str, 0},
                                                    {"volutpat"_str, 0},
                                                    {"In"_str, 0},
                                                    {"pellentesque"_str, 0},
                                                    {"auctor"_str, 0},
                                                    {"vulputate"_str, 0},
                                                    {"Suspendisse"_str, 0},
                                                    {"rhoncus"_str, 0},
                                                    {"magna"_str, 0},
                                                    {"quis"_str, 0},
                                                    {"tincidunt"_str, 0},
                                                    {"gravida"_str, 0},
                                                    {"libero"_str, 0},
                                                    {"ex"_str, 0},
                                                    {"egestas"_str, 0},
                                                    {"diam"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"hendrerit"_str, 0},
                                                    {"odio"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"velit"_str, 0},
                                                    {"Proin"_str, 0},
                                                    {"quis"_str, 0},
                                                    {"nulla"_str, 0},
                                                    {"placerat"_str, 0},
                                                    {"sagittis"_str, 0},
                                                    {"augue"_str, 0},
                                                    {"in"_str, 0},
                                                    {"mattis"_str, 0},
                                                    {"tortor"_str, 0},
                                                    {"Aliquam"_str, 0},
                                                    {"accumsan"_str, 0},
                                                    {"metus"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"nisl"_str, 0},
                                                    {"hendrerit"_str, 0},
                                                    {"non"_str, 0},
                                                    {"hendrerit"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"commodo"_str, 0},
                                                    {"Suspendisse"_str, 0},
                                                    {"bibendum"_str, 0},
                                                    {"euismod"_str, 0},
                                                    {"gravida"_str, 0},
                                                    {"Ut"_str, 0},
                                                    {"nisi"_str, 0},
                                                    {"libero"_str, 0},
                                                    {"facilisis"_str, 0},
                                                    {"nec"_str, 0},
                                                    {"erat"_str, 0},
                                                    {"a"_str, 0},
                                                    {"tempus"_str, 0},
                                                    {"ullamcorper"_str, 0},
                                                    {"risus"_str, 0},
                                                    {"Curabitur"_str, 0},
                                                    {"tortor"_str, 0},
                                                    {"mi"_str, 0},
                                                    {"suscipit"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"odio"_str, 0},
                                                    {"quis"_str, 0},
                                                    {"egestas"_str, 0},
                                                    {"suscipit"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"Nam"_str, 0},
                                                    {"id"_str, 0},
                                                    {"velit"_str, 0},
                                                    {"vel"_str, 0},
                                                    {"eros"_str, 0},
                                                    {"pharetra"_str, 0},
                                                    {"tristique"_str, 0},
                                                    {"nec"_str, 0},
                                                    {"at"_str, 0},
                                                    {"orci"_str, 0},
                                                    {"Lorem"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"dolor"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"adipiscing"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"Nulla"_str, 0},
                                                    {"dignissim"_str, 0},
                                                    {"magna"_str, 0},
                                                    {"quis"_str, 0},
                                                    {"venenatis"_str, 0},
                                                    {"ornare"_str, 0},
                                                    {"libero"_str, 0},
                                                    {"turpis"_str, 0},
                                                    {"sagittis"_str, 0},
                                                    {"tortor"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"posuere"_str, 0},
                                                    {"dui"_str, 0},
                                                    {"nunc"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"erat"_str, 0},
                                                    {"Aenean"_str, 0},
                                                    {"ullamcorper"_str, 0},
                                                    {"interdum"_str, 0},
                                                    {"mi"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"auctor"_str, 0},
                                                    {"Pellentesque"_str, 0},
                                                    {"tempor"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"urna"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"imperdiet"_str, 0},
                                                    {"orci"_str, 0},
                                                    {"iaculis"_str, 0},
                                                    {"In"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"fringilla"_str, 0},
                                                    {"augue"_str, 0},
                                                    {"Curabitur"_str, 0},
                                                    {"maximus"_str, 0},
                                                    {"nunc"_str, 0},
                                                    {"orci"_str, 0},
                                                    {"a"_str, 0},
                                                    {"elementum"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"blandit"_str, 0},
                                                    {"in"_str, 0},
                                                    {"Aliquam"_str, 0},
                                                    {"erat"_str, 0},
                                                    {"volutpat"_str, 0},
                                                    {"Donec"_str, 0},
                                                    {"feugiat"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"non"_str, 0},
                                                    {"scelerisque"_str, 0},
                                                    {"euismod"_str, 0},
                                                    {"Suspendisse"_str, 0},
                                                    {"sem"_str, 0},
                                                    {"diam"_str, 0},
                                                    {"consequat"_str, 0},
                                                    {"at"_str, 0},
                                                    {"tempus"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"lacinia"_str, 0},
                                                    {"et"_str, 0},
                                                    {"lectus"_str, 0},
                                                    {"Quisque"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"urna"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"vestibulum"_str, 0},
                                                    {"nisl"_str, 0},
                                                    {"at"_str, 0},
                                                    {"venenatis"_str, 0},
                                                    {"dui"_str, 0},
                                                    {"Donec"_str, 0},
                                                    {"maximus"_str, 0},
                                                    {"aliquam"_str, 0},
                                                    {"ornare"_str, 0},
                                                    {"Pellentesque"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"orci"_str, 0},
                                                    {"non"_str, 0},
                                                    {"ligula"_str, 0},
                                                    {"hendrerit"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"Vestibulum"_str, 0},
                                                    {"varius"_str, 0},
                                                    {"eros"_str, 0},
                                                    {"odio"_str, 0},
                                                    {"consequat"_str, 0},
                                                    {"aliquam"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"interdum"_str, 0},
                                                    {"condimentum"_str, 0},
                                                    {"Curabitur"_str, 0},
                                                    {"blandit"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"et"_str, 0},
                                                    {"varius"_str, 0},
                                                    {"Pellentesque"_str, 0},
                                                    {"lobortis"_str, 0},
                                                    {"nibh"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"consequat"_str, 0},
                                                    {"feugiat"_str, 0},
                                                    {"Phasellus"_str, 0},
                                                    {"ultricies"_str, 0},
                                                    {"enim"_str, 0},
                                                    {"purus"_str, 0},
                                                    {"et"_str, 0},
                                                    {"ultricies"_str, 0},
                                                    {"nulla"_str, 0},
                                                    {"molestie"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"Pellentesque"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"urna"_str, 0},
                                                    {"tristique"_str, 0},
                                                    {"a"_str, 0},
                                                    {"urna"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"fringilla"_str, 0},
                                                    {"consequat"_str, 0},
                                                    {"orci"_str, 0},
                                                    {"Nulla"_str, 0},
                                                    {"pharetra"_str, 0},
                                                    {"commodo"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"vel"_str, 0},
                                                    {"volutpat"_str, 0},
                                                    {"tellus"_str, 0},
                                                    {"porttitor"_str, 0},
                                                    {"a"_str, 0},
                                                    {"Praesent"_str, 0},
                                                    {"id"_str, 0},
                                                    {"augue"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"Mauris"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"aliquet"_str, 0},
                                                    {"dui"_str, 0},
                                                    {"Vestibulum"_str, 0},
                                                    {"fringilla"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"tincidunt"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"convallis"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"Aenean"_str, 0},
                                                    {"vel"_str, 0},
                                                    {"lacinia"_str, 0},
                                                    {"mauris"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"fringilla"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"Nulla"_str, 0},
                                                    {"pulvinar"_str, 0},
                                                    {"dolor"_str, 0},
                                                    {"quis"_str, 0},
                                                    {"dui"_str, 0},
                                                    {"aliquet"_str, 0},
                                                    {"elementum"_str, 0},
                                                    {"Interdum"_str, 0},
                                                    {"et"_str, 0},
                                                    {"malesuada"_str, 0},
                                                    {"fames"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"primis"_str, 0},
                                                    {"in"_str, 0},
                                                    {"faucibus"_str, 0},
                                                    {"Curabitur"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"ex"_str, 0},
                                                    {"libero"_str, 0},
                                                    {"In"_str, 0},
                                                    {"hac"_str, 0},
                                                    {"habitasse"_str, 0},
                                                    {"platea"_str, 0},
                                                    {"dictumst"_str, 0},
                                                    {"Donec"_str, 0},
                                                    {"tincidunt"_str, 0},
                                                    {"bibendum"_str, 0},
                                                    {"urna"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"suscipit"_str, 0},
                                                    {"Quisque"_str, 0},
                                                    {"at"_str, 0},
                                                    {"purus"_str, 0},
                                                    {"nec"_str, 0},
                                                    {"tellus"_str, 0},
                                                    {"sagittis"_str, 0},
                                                    {"fringilla"_str, 0},
                                                    {"In"_str, 0},
                                                    {"hac"_str, 0},
                                                    {"habitasse"_str, 0},
                                                    {"platea"_str, 0},
                                                    {"dictumst"_str, 0},
                                                    {"Curabitur"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"et"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"dapibus"_str, 0},
                                                    {"sagittis"_str, 0},
                                                    {"In"_str, 0},
                                                    {"tempor"_str, 0},
                                                    {"nisi"_str, 0},
                                                    {"in"_str, 0},
                                                    {"cursus"_str, 0},
                                                    {"tincidunt"_str, 0},
                                                    {"nisl"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"accumsan"_str, 0},
                                                    {"libero"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"imperdiet"_str, 0},
                                                    {"arcu"_str, 0},
                                                    {"massa"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"nisl"_str, 0},
                                                    {"In"_str, 0},
                                                    {"et"_str, 0},
                                                    {"vulputate"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"ullamcorper"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"Fusce"_str, 0},
                                                    {"quis"_str, 0},
                                                    {"augue"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"nunc"_str, 0},
                                                    {"feugiat"_str, 0},
                                                    {"ultrices"_str, 0},
                                                    {"Nulla"_str, 0},
                                                    {"est"_str, 0},
                                                    {"augue"_str, 0},
                                                    {"pretium"_str, 0},
                                                    {"et"_str, 0},
                                                    {"volutpat"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"mattis"_str, 0},
                                                    {"in"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"Aenean"_str, 0},
                                                    {"molestie"_str, 0},
                                                    {"magna"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"Ut"_str, 0},
                                                    {"lorem"_str, 0},
                                                    {"sapien"_str, 0},
                                                    {"placerat"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"porta"_str, 0},
                                                    {"et"_str, 0},
                                                    {"congue"_str, 0},
                                                    {"blandit"_str, 0},
                                                    {"neque"_str, 0},
                                                    {"Mauris"_str, 0},
                                                    {"tristique"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"a"_str, 0},
                                                    {"ullamcorper"_str, 0},
                                                    {"dignissim"_str, 0},
                                                    {"sapien"_str, 0},
                                                    {"nisi"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"diam"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"tincidunt"_str, 0},
                                                    {"nisl"_str, 0},
                                                    {"est"_str, 0},
                                                    {"id"_str, 0},
                                                    {"enim"_str, 0},
                                                    {"Quisque"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"nulla"_str, 0},
                                                    {"velit"_str, 0},
                                                    {"Vestibulum"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"libero"_str, 0},
                                                    {"turpis"_str, 0},
                                                    {"Phasellus"_str, 0},
                                                    {"tristique"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"non"_str, 0},
                                                    {"semper"_str, 0},
                                                    {"laoreet"_str, 0},
                                                    {"Maecenas"_str, 0},
                                                    {"vehicula"_str, 0},
                                                    {"congue"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"ultrices"_str, 0},
                                                    {"Sed"_str, 0},
                                                    {"ex"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"scelerisque"_str, 0},
                                                    {"non"_str, 0},
                                                    {"ligula"_str, 0},
                                                    {"et"_str, 0},
                                                    {"vestibulum"_str, 0},
                                                    {"tristique"_str, 0},
                                                    {"est"_str, 0},
                                                    {"In"_str, 0},
                                                    {"egestas"_str, 0},
                                                    {"porttitor"_str, 0},
                                                    {"tortor"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"tempor"_str, 0},
                                                    {"enim"_str, 0},
                                                    {"dictum"_str, 0},
                                                    {"non"_str, 0},
                                                    {"Vestibulum"_str, 0},
                                                    {"tincidunt"_str, 0},
                                                    {"leo"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"consequat"_str, 0},
                                                    {"pulvinar"_str, 0},
                                                    {"Morbi"_str, 0},
                                                    {"dictum"_str, 0},
                                                    {"mi"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"bibendum"_str, 0},
                                                    {"ullamcorper"_str, 0},
                                                    {"Integer"_str, 0},
                                                    {"nunc"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"varius"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"libero"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"viverra"_str, 0},
                                                    {"commodo"_str, 0},
                                                    {"ligula"_str, 0},
                                                    {"Pellentesque"_str, 0},
                                                    {"egestas"_str, 0},
                                                    {"scelerisque"_str, 0},
                                                    {"orci"_str, 0},
                                                    {"id"_str, 0},
                                                    {"interdum"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"tristique"_str, 0},
                                                    {"et"_str, 0},
                                                    {"Fusce"_str, 0},
                                                    {"non"_str, 0},
                                                    {"leo"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"Sed"_str, 0},
                                                    {"enim"_str, 0},
                                                    {"dui"_str, 0},
                                                    {"malesuada"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"eros"_str, 0},
                                                    {"non"_str, 0},
                                                    {"tristique"_str, 0},
                                                    {"dictum"_str, 0},
                                                    {"metus"_str, 0},
                                                    {"In"_str, 0},
                                                    {"non"_str, 0},
                                                    {"lectus"_str, 0},
                                                    {"feugiat"_str, 0},
                                                    {"pulvinar"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"interdum"_str, 0},
                                                    {"ligula"_str, 0},
                                                    {"Quisque"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"accumsan"_str, 0},
                                                    {"aliquet"_str, 0},
                                                    {"risus"_str, 0},
                                                    {"id"_str, 0},
                                                    {"dictum"_str, 0},
                                                    {"nibh"_str, 0},
                                                    {"Orci"_str, 0},
                                                    {"varius"_str, 0},
                                                    {"natoque"_str, 0},
                                                    {"penatibus"_str, 0},
                                                    {"et"_str, 0},
                                                    {"magnis"_str, 0},
                                                    {"dis"_str, 0},
                                                    {"parturient"_str, 0},
                                                    {"montes"_str, 0},
                                                    {"nascetur"_str, 0},
                                                    {"ridiculus"_str, 0},
                                                    {"mus"_str, 0},
                                                    {"Vestibulum"_str, 0},
                                                    {"sodales"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"non"_str, 0},
                                                    {"luctus"_str, 0},
                                                    {"egestas"_str, 0},
                                                    {"Ut"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"neque"_str, 0},
                                                    {"Nulla"_str, 0},
                                                    {"nec"_str, 0},
                                                    {"arcu"_str, 0},
                                                    {"interdum"_str, 0},
                                                    {"auctor"_str, 0},
                                                    {"nisi"_str, 0},
                                                    {"non"_str, 0},
                                                    {"posuere"_str, 0},
                                                    {"dui"_str, 0},
                                                    {"Aliquam"_str, 0},
                                                    {"sodales"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"diam"_str, 0},
                                                    {"gravida"_str, 0},
                                                    {"porta"_str, 0},
                                                    {"Donec"_str, 0},
                                                    {"iaculis"_str, 0},
                                                    {"massa"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"nulla"_str, 0},
                                                    {"ultrices"_str, 0},
                                                    {"id"_str, 0},
                                                    {"pretium"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"sodales"_str, 0},
                                                    {"Curabitur"_str, 0},
                                                    {"dignissim"_str, 0},
                                                    {"purus"_str, 0},
                                                    {"ex"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"finibus"_str, 0},
                                                    {"nisi"_str, 0},
                                                    {"volutpat"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"Suspendisse"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"nibh"_str, 0},
                                                    {"non"_str, 0},
                                                    {"odio"_str, 0},
                                                    {"varius"_str, 0},
                                                    {"porttitor"_str, 0},
                                                    {"Nulla"_str, 0},
                                                    {"elementum"_str, 0},
                                                    {"ullamcorper"_str, 0},
                                                    {"ultrices"_str, 0},
                                                    {"Sed"_str, 0},
                                                    {"mattis"_str, 0},
                                                    {"purus"_str, 0},
                                                    {"libero"_str, 0},
                                                    {"non"_str, 0},
                                                    {"condimentum"_str, 0},
                                                    {"leo"_str, 0},
                                                    {"dapibus"_str, 0},
                                                    {"tristique"_str, 0},
                                                    {"In"_str, 0},
                                                    {"malesuada"_str, 0},
                                                    {"eleifend"_str, 0},
                                                    {"tortor"_str, 0},
                                                    {"non"_str, 0},
                                                    {"ornare"_str, 0},
                                                    {"enim"_str, 0},
                                                    {"fringilla"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"Sed"_str, 0},
                                                    {"dolor"_str, 0},
                                                    {"leo"_str, 0},
                                                    {"commodo"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"felis"_str, 0},
                                                    {"viverra"_str, 0},
                                                    {"dapibus"_str, 0},
                                                    {"convallis"_str, 0},
                                                    {"tellus"_str, 0},
                                                    {"Phasellus"_str, 0},
                                                    {"rutrum"_str, 0},
                                                    {"volutpat"_str, 0},
                                                    {"leo"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"imperdiet"_str, 0},
                                                    {"neque"_str, 0},
                                                    {"Cras"_str, 0},
                                                    {"libero"_str, 0},
                                                    {"orci"_str, 0},
                                                    {"feugiat"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"enim"_str, 0},
                                                    {"a"_str, 0},
                                                    {"rutrum"_str, 0},
                                                    {"egestas"_str, 0},
                                                    {"felis"_str, 0},
                                                    {"Curabitur"_str, 0},
                                                    {"vel"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"eros"_str, 0},
                                                    {"vestibulum"_str, 0},
                                                    {"tempor"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"quis"_str, 0},
                                                    {"purus"_str, 0},
                                                    {"Aliquam"_str, 0},
                                                    {"condimentum"_str, 0},
                                                    {"rhoncus"_str, 0},
                                                    {"facilisis"_str, 0},
                                                    {"Nulla"_str, 0},
                                                    {"euismod"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"lorem"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"laoreet"_str, 0},
                                                    {"orci"_str, 0},
                                                    {"auctor"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"Cras"_str, 0},
                                                    {"gravida"_str, 0},
                                                    {"risus"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"mauris"_str, 0},
                                                    {"scelerisque"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"pretium"_str, 0},
                                                    {"ante"_str, 0},
                                                    {"malesuada"_str, 0},
                                                    {"Nulla"_str, 0},
                                                    {"facilisis"_str, 0},
                                                    {"molestie"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"pellentesque"_str, 0},
                                                    {"Morbi"_str, 0},
                                                    {"id"_str, 0},
                                                    {"odio"_str, 0},
                                                    {"nunc"_str, 0},
                                                    {"Cras"_str, 0},
                                                    {"fermentum"_str, 0},
                                                    {"augue"_str, 0},
                                                    {"a"_str, 0},
                                                    {"felis"_str, 0},
                                                    {"aliquet"_str, 0},
                                                    {"placerat"_str, 0},
                                                    {"Mauris"_str, 0},
                                                    {"lorem"_str, 0},
                                                    {"ex"_str, 0},
                                                    {"fermentum"_str, 0},
                                                    {"vel"_str, 0},
                                                    {"euismod"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"fermentum"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"nulla"_str, 0},
                                                    {"Mauris"_str, 0},
                                                    {"quis"_str, 0},
                                                    {"pulvinar"_str, 0},
                                                    {"lorem"_str, 0},
                                                    {"in"_str, 0},
                                                    {"dapibus"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"Nunc"_str, 0},
                                                    {"et"_str, 0},
                                                    {"venenatis"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"tincidunt"_str, 0},
                                                    {"rhoncus"_str, 0},
                                                    {"sem"_str, 0},
                                                    {"Lorem"_str, 0},
                                                    {"ipsum"_str, 0},
                                                    {"dolor"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"adipiscing"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"Aenean"_str, 0},
                                                    {"dictum"_str, 0},
                                                    {"volutpat"_str, 0},
                                                    {"leo"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"commodo"_str, 0},
                                                    {"nisl"_str, 0},
                                                    {"eleifend"_str, 0},
                                                    {"eleifend"_str, 0},
                                                    {"Donec"_str, 0},
                                                    {"id"_str, 0},
                                                    {"elit"_str, 0},
                                                    {"viverra"_str, 0},
                                                    {"bibendum"_str, 0},
                                                    {"mi"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"tincidunt"_str, 0},
                                                    {"dui"_str, 0},
                                                    {"Mauris"_str, 0},
                                                    {"vel"_str, 0},
                                                    {"lacinia"_str, 0},
                                                    {"est"_str, 0},
                                                    {"Mauris"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"scelerisque"_str, 0},
                                                    {"diam"_str, 0},
                                                    {"non"_str, 0},
                                                    {"fringilla"_str, 0},
                                                    {"nisi"_str, 0},
                                                    {"Cras"_str, 0},
                                                    {"iaculis"_str, 0},
                                                    {"neque"_str, 0},
                                                    {"leo"_str, 0},
                                                    {"pretium"_str, 0},
                                                    {"pellentesque"_str, 0},
                                                    {"est"_str, 0},
                                                    {"eleifend"_str, 0},
                                                    {"a"_str, 0},
                                                    {"Integer"_str, 0},
                                                    {"egestas"_str, 0},
                                                    {"rutrum"_str, 0},
                                                    {"semper"_str, 0},
                                                    {"Mauris"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"iaculis"_str, 0},
                                                    {"diam"_str, 0},
                                                    {"placerat"_str, 0},
                                                    {"luctus"_str, 0},
                                                    {"arcu"_str, 0},
                                                    {"Praesent"_str, 0},
                                                    {"pellentesque"_str, 0},
                                                    {"egestas"_str, 0},
                                                    {"massa"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"varius"_str, 0},
                                                    {"Aliquam"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"leo"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"turpis"_str, 0},
                                                    {"lobortis"_str, 0},
                                                    {"pretium"_str, 0},
                                                    {"id"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"tellus"_str, 0},
                                                    {"Phasellus"_str, 0},
                                                    {"ut"_str, 0},
                                                    {"turpis"_str, 0},
                                                    {"ac"_str, 0},
                                                    {"libero"_str, 0},
                                                    {"pretium"_str, 0},
                                                    {"eleifend"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"tortor"_str, 0},
                                                    {"Curabitur"_str, 0},
                                                    {"dictum"_str, 0},
                                                    {"id"_str, 0},
                                                    {"nulla"_str, 0},
                                                    {"non"_str, 0},
                                                    {"vehicula"_str, 0},
                                                    {"Quisque"_str, 0},
                                                    {"nec"_str, 0},
                                                    {"purus"_str, 0},
                                                    {"vulputate"_str, 0},
                                                    {"felis"_str, 0},
                                                    {"pellentesque"_str, 0},
                                                    {"lacinia"_str, 0},
                                                    {"Donec"_str, 0},
                                                    {"vel"_str, 0},
                                                    {"consectetur"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"Aenean"_str, 0},
                                                    {"lectus"_str, 0},
                                                    {"sapien"_str, 0},
                                                    {"tincidunt"_str, 0},
                                                    {"a"_str, 0},
                                                    {"tortor"_str, 0},
                                                    {"sed"_str, 0},
                                                    {"fermentum"_str, 0},
                                                    {"tristique"_str, 0},
                                                    {"nibh"_str, 0},
                                                    {"Ut"_str, 0},
                                                    {"eget"_str, 0},
                                                    {"justo"_str, 0},
                                                    {"lorem"_str, 0},
                                                    {"Morbi"_str, 0},
                                                    {"efficitur"_str, 0},
                                                    {"elementum"_str, 0},
                                                    {"efficitur"_str, 0},
                                                    {"Vestibulum"_str, 0},
                                                    {"auctor"_str, 0},
                                                    {"sem"_str, 0},
                                                    {"vel"_str, 0},
                                                    {"efficitur"_str, 0},
                                                    {"auctor"_str, 0},
                                                    {"Sed"_str, 0},
                                                    {"diam"_str, 0},
                                                    {"nisi"_str, 0},
                                                    {"dignissim"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"lectus"_str, 0},
                                                    {"sit"_str, 0},
                                                    {"amet"_str, 0},
                                                    {"lacinia"_str, 0},
                                                    {"pulvinar"_str, 0},
                                                    {"nisi"_str, 0},
                                                    {"Pellentesque"_str, 0},
                                                    {"malesuada"_str, 0},
                                                    {"dolor"_str, 0},
                                                    {"vitae"_str, 0},
                                                    {"egestas"_str, 0},
                                                    {"tempus"_str, 0},
                                                    {"Mauris"_str, 0},
                                                    {"placerat"_str, 0},
                                                    {"ex"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"est"_str, 0},
                                                    {"posuere"_str, 0},
                                                    {"dapibus"_str, 0},
                                                    {"Pellentesque"_str, 0},
                                                    {"bibendum"_str, 0},
                                                    {"dui"_str, 0},
                                                    {"nec"_str, 0},
                                                    {"sodales"_str, 0},
                                                    {"congue"_str, 0},
                                                    {"lacus"_str, 0},
                                                    {"nibh"_str, 0},
                                                    {"pulvinar"_str, 0},
                                                    {"diam"_str, 0},
                                                    {"eu"_str, 0},
                                                    {"tempus"_str, 0},
                                                    {"sapien"_str, 0},
                                                    {"eros"_str, 0},
                                                    {"quis"_str, 0},
                                                    {"arcu"_str, 0},
                                                    {"Class"_str, 0},
                                                    {"aptent"_str, 0},
                                                    {"taciti"_str, 0},
                                                    {"sociosqu"_str, 0},
                                                    {"ad"_str, 0}};

static void BM_Map16(benchmark::State &state)
{
  StrMap<int> map;
  int               count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      map.insert(dp.v0, dp.v1).unwrap();
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

static void BM_Map8(benchmark::State &state)
{
  StrMap<int, u8> map;
  int                   count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      map.insert(dp.v0, dp.v1).unwrap();
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

static void BM_Map32(benchmark::State &state)
{
  StrMap<int, u32> map;
  int                    count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      map.insert(dp.v0, dp.v1).unwrap();
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

static void BM_Map64(benchmark::State &state)
{
  StrMap<int, u64> map;
  int                    count = 0;
  for (auto _ : state)
  {
    for (auto &dp : DATASET)
    {
      map.insert(dp.v0, dp.v1).unwrap();
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

  pointer allocate(size_type s, void const * = 0)
  {
    pointer temp;
    if (!default_allocator.nalloc(s, temp))
    {
      throw std::bad_alloc();
    }
    return temp;
  }

  void deallocate(pointer p, size_type s)
  {
    default_allocator.ndealloc(p, s);
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

static void BM_StdMap(benchmark::State &state)
{
  std::unordered_map<Span<char const>, int, StrHasher, StrEq,
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
    return std::lexicographical_compare(a.pbegin(), a.pend(), b.pbegin(), b.pend());
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

static void BM_StdMapDefaultHash(benchmark::State &state)
{
  std::unordered_map<Span<char const>, int, std::hash<Span<char const>>, StrEq,
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

static void BM_StdMapDefaultHashDefaultAlloc(benchmark::State &state)
{
  std::unordered_map<Span<char const>, int, std::hash<Span<char const>>, StrEq>
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
BENCHMARK(BM_Map8)->Iterations(ITERATIONS);
BENCHMARK(BM_Map16)->Iterations(ITERATIONS);
BENCHMARK(BM_Map32)->Iterations(ITERATIONS);
BENCHMARK(BM_Map64)->Iterations(ITERATIONS);
BENCHMARK(BM_StdMap)->Iterations(ITERATIONS);
BENCHMARK(BM_StdMapDefaultHash)->Iterations(ITERATIONS);
BENCHMARK(BM_StdMapDefaultHashDefaultAlloc)->Iterations(ITERATIONS);
BENCHMARK(BM_StdOrderedMapDefaultAlloc)->Iterations(ITERATIONS);
BENCHMARK_MAIN();