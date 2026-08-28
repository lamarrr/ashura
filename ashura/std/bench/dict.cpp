/// SPDX-License-Identifier: MIT
#include "ashura/std/dict.hpp"
#include "ashura/std/types.hpp"
#include "stdint.h"
#include <algorithm>
#include <benchmark/benchmark.h>
#include <map>
#include <unordered_map>

using namespace ash;

constexpr Str DATASET[] = {
  "Lorem"_s,
  "ipsum"_s,
  "dolor"_s,
  "sit"_s,
  "amet"_s,
  "consectetur"_s,
  "adipiscing"_s,
  "elit"_s,
  "Nullam"_s,
  "ultricies"_s,
  "purus"_s,
  "facilisis"_s,
  "orci"_s,
  "euismod"_s,
  "eleifend"_s,
  "Pellentesque"_s,
  "bibendum"_s,
  "pretium"_s,
  "quam"_s,
  "et"_s,
  "gravida"_s,
  "Proin"_s,
  "tortor"_s,
  "urna"_s,
  "convallis"_s,
  "eget"_s,
  "neque"_s,
  "sed"_s,
  "commodo"_s,
  "consectetur"_s,
  "magna"_s,
  "Praesent"_s,
  "ac"_s,
  "nisl"_s,
  "eu"_s,
  "purus"_s,
  "pretium"_s,
  "ultrices"_s,
  "vitae"_s,
  "ut"_s,
  "ante"_s,
  "Etiam"_s,
  "sit"_s,
  "amet"_s,
  "sapien"_s,
  "elit"_s,
  "Morbi"_s,
  "bibendum"_s,
  "consectetur"_s,
  "dolor"_s,
  "convallis"_s,
  "egestas"_s,
  "Interdum"_s,
  "et"_s,
  "malesuada"_s,
  "fames"_s,
  "ac"_s,
  "ante"_s,
  "ipsum"_s,
  "primis"_s,
  "in"_s,
  "faucibus"_s,
  "Phasellus"_s,
  "condimentum"_s,
  "hendrerit"_s,
  "tellus"_s,
  "Nam"_s,
  "eleifend"_s,
  "justo"_s,
  "at"_s,
  "ultrices"_s,
  "sodales"_s,
  "Aliquam"_s,
  "sit"_s,
  "amet"_s,
  "ante"_s,
  "in"_s,
  "ligula"_s,
  "elementum"_s,
  "dignissim"_s,
  "Integer"_s,
  "in"_s,
  "justo"_s,
  "in"_s,
  "ipsum"_s,
  "varius"_s,
  "imperdiet"_s,
  "a"_s,
  "sed"_s,
  "dui"_s,
  "Class"_s,
  "aptent"_s,
  "taciti"_s,
  "sociosqu"_s,
  "ad"_s,
  "litora"_s,
  "torquent"_s,
  "per"_s,
  "conubia"_s,
  "nostra"_s,
  "per"_s,
  "inceptos"_s,
  "himenaeos"_s,
  "Nunc"_s,
  "leo"_s,
  "eros"_s,
  "ullamcorper"_s,
  "vitae"_s,
  "fermentum"_s,
  "id"_s,
  "molestie"_s,
  "sit"_s,
  "amet"_s,
  "tellus"_s,
  "Sed"_s,
  "vel"_s,
  "nunc"_s,
  "sed"_s,
  "nulla"_s,
  "varius"_s,
  "porttitor"_s,
  "Vivamus"_s,
  "vitae"_s,
  "molestie"_s,
  "sapien"_s,
  "Sed"_s,
  "imperdiet"_s,
  "justo"_s,
  "mauris"_s,
  "sit"_s,
  "amet"_s,
  "elementum"_s,
  "metus"_s,
  "tempor"_s,
  "ac"_s,
  "Praesent"_s,
  "et"_s,
  "diam"_s,
  "et"_s,
  "orci"_s,
  "blandit"_s,
  "convallis"_s,
  "rutrum"_s,
  "eu"_s,
  "nisl"_s,
  "Donec"_s,
  "vulputate"_s,
  "hendrerit"_s,
  "nisi"_s,
  "sit"_s,
  "amet"_s,
  "rutrum"_s,
  "Nullam"_s,
  "faucibus"_s,
  "tincidunt"_s,
  "lectus"_s,
  "eu"_s,
  "gravida"_s,
  "est"_s,
  "faucibus"_s,
  "imperdiet"_s,
  "Nam"_s,
  "varius"_s,
  "vehicula"_s,
  "risus"_s,
  "ut"_s,
  "tempus"_s,
  "Aliquam"_s,
  "erat"_s,
  "volutpat"_s,
  "In"_s,
  "pellentesque"_s,
  "auctor"_s,
  "vulputate"_s,
  "Suspendisse"_s,
  "rhoncus"_s,
  "magna"_s,
  "quis"_s,
  "tincidunt"_s,
  "gravida"_s,
  "libero"_s,
  "ex"_s,
  "egestas"_s,
  "diam"_s,
  "eget"_s,
  "hendrerit"_s,
  "odio"_s,
  "ante"_s,
  "eget"_s,
  "velit"_s,
  "Proin"_s,
  "quis"_s,
  "nulla"_s,
  "placerat"_s,
  "sagittis"_s,
  "augue"_s,
  "in"_s,
  "mattis"_s,
  "tortor"_s,
  "Aliquam"_s,
  "accumsan"_s,
  "metus"_s,
  "eu"_s,
  "nisl"_s,
  "hendrerit"_s,
  "non"_s,
  "hendrerit"_s,
  "justo"_s,
  "commodo"_s,
  "Suspendisse"_s,
  "bibendum"_s,
  "euismod"_s,
  "gravida"_s,
  "Ut"_s,
  "nisi"_s,
  "libero"_s,
  "facilisis"_s,
  "nec"_s,
  "erat"_s,
  "a"_s,
  "tempus"_s,
  "ullamcorper"_s,
  "risus"_s,
  "Curabitur"_s,
  "tortor"_s,
  "mi"_s,
  "suscipit"_s,
  "sit"_s,
  "amet"_s,
  "odio"_s,
  "quis"_s,
  "egestas"_s,
  "suscipit"_s,
  "elit"_s,
  "Nam"_s,
  "id"_s,
  "velit"_s,
  "vel"_s,
  "eros"_s,
  "pharetra"_s,
  "tristique"_s,
  "nec"_s,
  "at"_s,
  "orci"_s,
  "Lorem"_s,
  "ipsum"_s,
  "dolor"_s,
  "sit"_s,
  "amet"_s,
  "consectetur"_s,
  "adipiscing"_s,
  "elit"_s,
  "Nulla"_s,
  "dignissim"_s,
  "magna"_s,
  "quis"_s,
  "venenatis"_s,
  "ornare"_s,
  "libero"_s,
  "turpis"_s,
  "sagittis"_s,
  "tortor"_s,
  "ut"_s,
  "posuere"_s,
  "dui"_s,
  "nunc"_s,
  "eu"_s,
  "erat"_s,
  "Aenean"_s,
  "ullamcorper"_s,
  "interdum"_s,
  "mi"_s,
  "eu"_s,
  "auctor"_s,
  "Pellentesque"_s,
  "tempor"_s,
  "elit"_s,
  "vitae"_s,
  "urna"_s,
  "consectetur"_s,
  "eu"_s,
  "imperdiet"_s,
  "orci"_s,
  "iaculis"_s,
  "In"_s,
  "eu"_s,
  "fringilla"_s,
  "augue"_s,
  "Curabitur"_s,
  "maximus"_s,
  "nunc"_s,
  "orci"_s,
  "a"_s,
  "elementum"_s,
  "ipsum"_s,
  "blandit"_s,
  "in"_s,
  "Aliquam"_s,
  "erat"_s,
  "volutpat"_s,
  "Donec"_s,
  "feugiat"_s,
  "ipsum"_s,
  "non"_s,
  "scelerisque"_s,
  "euismod"_s,
  "Suspendisse"_s,
  "sem"_s,
  "diam"_s,
  "consequat"_s,
  "at"_s,
  "tempus"_s,
  "eu"_s,
  "lacinia"_s,
  "et"_s,
  "lectus"_s,
  "Quisque"_s,
  "ac"_s,
  "urna"_s,
  "consectetur"_s,
  "vestibulum"_s,
  "nisl"_s,
  "at"_s,
  "venenatis"_s,
  "dui"_s,
  "Donec"_s,
  "maximus"_s,
  "aliquam"_s,
  "ornare"_s,
  "Pellentesque"_s,
  "sit"_s,
  "amet"_s,
  "orci"_s,
  "non"_s,
  "ligula"_s,
  "hendrerit"_s,
  "consectetur"_s,
  "Vestibulum"_s,
  "varius"_s,
  "eros"_s,
  "odio"_s,
  "consequat"_s,
  "aliquam"_s,
  "lacus"_s,
  "interdum"_s,
  "condimentum"_s,
  "Curabitur"_s,
  "blandit"_s,
  "ut"_s,
  "ante"_s,
  "et"_s,
  "varius"_s,
  "Pellentesque"_s,
  "lobortis"_s,
  "nibh"_s,
  "eu"_s,
  "consequat"_s,
  "feugiat"_s,
  "Phasellus"_s,
  "ultricies"_s,
  "enim"_s,
  "purus"_s,
  "et"_s,
  "ultricies"_s,
  "nulla"_s,
  "molestie"_s,
  "ac"_s,
  "Pellentesque"_s,
  "lacus"_s,
  "urna"_s,
  "tristique"_s,
  "a"_s,
  "urna"_s,
  "sed"_s,
  "fringilla"_s,
  "consequat"_s,
  "orci"_s,
  "Nulla"_s,
  "pharetra"_s,
  "commodo"_s,
  "ipsum"_s,
  "vel"_s,
  "volutpat"_s,
  "tellus"_s,
  "porttitor"_s,
  "a"_s,
  "Praesent"_s,
  "id"_s,
  "augue"_s,
  "lacus"_s,
  "Mauris"_s,
  "sed"_s,
  "aliquet"_s,
  "dui"_s,
  "Vestibulum"_s,
  "fringilla"_s,
  "lacus"_s,
  "elit"_s,
  "ac"_s,
  "tincidunt"_s,
  "justo"_s,
  "convallis"_s,
  "eget"_s,
  "Aenean"_s,
  "vel"_s,
  "lacinia"_s,
  "mauris"_s,
  "eu"_s,
  "fringilla"_s,
  "lacus"_s,
  "Nulla"_s,
  "pulvinar"_s,
  "dolor"_s,
  "quis"_s,
  "dui"_s,
  "aliquet"_s,
  "elementum"_s,
  "Interdum"_s,
  "et"_s,
  "malesuada"_s,
  "fames"_s,
  "ac"_s,
  "ante"_s,
  "ipsum"_s,
  "primis"_s,
  "in"_s,
  "faucibus"_s,
  "Curabitur"_s,
  "eu"_s,
  "ex"_s,
  "libero"_s,
  "In"_s,
  "hac"_s,
  "habitasse"_s,
  "platea"_s,
  "dictumst"_s,
  "Donec"_s,
  "tincidunt"_s,
  "bibendum"_s,
  "urna"_s,
  "sit"_s,
  "amet"_s,
  "suscipit"_s,
  "Quisque"_s,
  "at"_s,
  "purus"_s,
  "nec"_s,
  "tellus"_s,
  "sagittis"_s,
  "fringilla"_s,
  "In"_s,
  "hac"_s,
  "habitasse"_s,
  "platea"_s,
  "dictumst"_s,
  "Curabitur"_s,
  "eget"_s,
  "justo"_s,
  "et"_s,
  "ante"_s,
  "dapibus"_s,
  "sagittis"_s,
  "In"_s,
  "tempor"_s,
  "nisi"_s,
  "in"_s,
  "cursus"_s,
  "tincidunt"_s,
  "nisl"_s,
  "ipsum"_s,
  "accumsan"_s,
  "libero"_s,
  "eu"_s,
  "imperdiet"_s,
  "arcu"_s,
  "massa"_s,
  "eu"_s,
  "nisl"_s,
  "In"_s,
  "et"_s,
  "vulputate"_s,
  "ante"_s,
  "eu"_s,
  "ullamcorper"_s,
  "justo"_s,
  "Fusce"_s,
  "quis"_s,
  "augue"_s,
  "eu"_s,
  "nunc"_s,
  "feugiat"_s,
  "ultrices"_s,
  "Nulla"_s,
  "est"_s,
  "augue"_s,
  "pretium"_s,
  "et"_s,
  "volutpat"_s,
  "ac"_s,
  "mattis"_s,
  "in"_s,
  "ante"_s,
  "Aenean"_s,
  "molestie"_s,
  "magna"_s,
  "lacus"_s,
  "Ut"_s,
  "lorem"_s,
  "sapien"_s,
  "placerat"_s,
  "sit"_s,
  "amet"_s,
  "porta"_s,
  "et"_s,
  "congue"_s,
  "blandit"_s,
  "neque"_s,
  "Mauris"_s,
  "tristique"_s,
  "ipsum"_s,
  "a"_s,
  "ullamcorper"_s,
  "dignissim"_s,
  "sapien"_s,
  "nisi"_s,
  "consectetur"_s,
  "diam"_s,
  "vitae"_s,
  "tincidunt"_s,
  "nisl"_s,
  "est"_s,
  "id"_s,
  "enim"_s,
  "Quisque"_s,
  "ut"_s,
  "nulla"_s,
  "velit"_s,
  "Vestibulum"_s,
  "sit"_s,
  "amet"_s,
  "libero"_s,
  "turpis"_s,
  "Phasellus"_s,
  "tristique"_s,
  "justo"_s,
  "non"_s,
  "semper"_s,
  "laoreet"_s,
  "Maecenas"_s,
  "vehicula"_s,
  "congue"_s,
  "ante"_s,
  "sed"_s,
  "ultrices"_s,
  "Sed"_s,
  "ex"_s,
  "elit"_s,
  "scelerisque"_s,
  "non"_s,
  "ligula"_s,
  "et"_s,
  "vestibulum"_s,
  "tristique"_s,
  "est"_s,
  "In"_s,
  "egestas"_s,
  "porttitor"_s,
  "tortor"_s,
  "eget"_s,
  "tempor"_s,
  "enim"_s,
  "dictum"_s,
  "non"_s,
  "Vestibulum"_s,
  "tincidunt"_s,
  "leo"_s,
  "sed"_s,
  "consequat"_s,
  "pulvinar"_s,
  "Morbi"_s,
  "dictum"_s,
  "mi"_s,
  "sit"_s,
  "amet"_s,
  "bibendum"_s,
  "ullamcorper"_s,
  "Integer"_s,
  "nunc"_s,
  "ipsum"_s,
  "varius"_s,
  "sit"_s,
  "amet"_s,
  "libero"_s,
  "sit"_s,
  "amet"_s,
  "viverra"_s,
  "commodo"_s,
  "ligula"_s,
  "Pellentesque"_s,
  "egestas"_s,
  "scelerisque"_s,
  "orci"_s,
  "id"_s,
  "interdum"_s,
  "elit"_s,
  "tristique"_s,
  "et"_s,
  "Fusce"_s,
  "non"_s,
  "leo"_s,
  "justo"_s,
  "Sed"_s,
  "enim"_s,
  "dui"_s,
  "malesuada"_s,
  "sed"_s,
  "eros"_s,
  "non"_s,
  "tristique"_s,
  "dictum"_s,
  "metus"_s,
  "In"_s,
  "non"_s,
  "lectus"_s,
  "feugiat"_s,
  "pulvinar"_s,
  "elit"_s,
  "vitae"_s,
  "interdum"_s,
  "ligula"_s,
  "Quisque"_s,
  "ac"_s,
  "justo"_s,
  "accumsan"_s,
  "aliquet"_s,
  "risus"_s,
  "id"_s,
  "dictum"_s,
  "nibh"_s,
  "Orci"_s,
  "varius"_s,
  "natoque"_s,
  "penatibus"_s,
  "et"_s,
  "magnis"_s,
  "dis"_s,
  "parturient"_s,
  "montes"_s,
  "nascetur"_s,
  "ridiculus"_s,
  "mus"_s,
  "Vestibulum"_s,
  "sodales"_s,
  "lacus"_s,
  "non"_s,
  "luctus"_s,
  "egestas"_s,
  "Ut"_s,
  "sed"_s,
  "consectetur"_s,
  "neque"_s,
  "Nulla"_s,
  "nec"_s,
  "arcu"_s,
  "interdum"_s,
  "auctor"_s,
  "nisi"_s,
  "non"_s,
  "posuere"_s,
  "dui"_s,
  "Aliquam"_s,
  "sodales"_s,
  "lacus"_s,
  "eget"_s,
  "diam"_s,
  "gravida"_s,
  "porta"_s,
  "Donec"_s,
  "iaculis"_s,
  "massa"_s,
  "ac"_s,
  "nulla"_s,
  "ultrices"_s,
  "id"_s,
  "pretium"_s,
  "ante"_s,
  "sodales"_s,
  "Curabitur"_s,
  "dignissim"_s,
  "purus"_s,
  "ex"_s,
  "ac"_s,
  "finibus"_s,
  "nisi"_s,
  "volutpat"_s,
  "eu"_s,
  "Suspendisse"_s,
  "eu"_s,
  "nibh"_s,
  "non"_s,
  "odio"_s,
  "varius"_s,
  "porttitor"_s,
  "Nulla"_s,
  "elementum"_s,
  "ullamcorper"_s,
  "ultrices"_s,
  "Sed"_s,
  "mattis"_s,
  "purus"_s,
  "libero"_s,
  "non"_s,
  "condimentum"_s,
  "leo"_s,
  "dapibus"_s,
  "tristique"_s,
  "In"_s,
  "malesuada"_s,
  "eleifend"_s,
  "tortor"_s,
  "non"_s,
  "ornare"_s,
  "enim"_s,
  "fringilla"_s,
  "eget"_s,
  "Sed"_s,
  "dolor"_s,
  "leo"_s,
  "commodo"_s,
  "sit"_s,
  "amet"_s,
  "felis"_s,
  "viverra"_s,
  "dapibus"_s,
  "convallis"_s,
  "tellus"_s,
  "Phasellus"_s,
  "rutrum"_s,
  "volutpat"_s,
  "leo"_s,
  "ut"_s,
  "imperdiet"_s,
  "neque"_s,
  "Cras"_s,
  "libero"_s,
  "orci"_s,
  "feugiat"_s,
  "vitae"_s,
  "enim"_s,
  "a"_s,
  "rutrum"_s,
  "egestas"_s,
  "felis"_s,
  "Curabitur"_s,
  "vel"_s,
  "ipsum"_s,
  "eget"_s,
  "eros"_s,
  "vestibulum"_s,
  "tempor"_s,
  "sit"_s,
  "amet"_s,
  "quis"_s,
  "purus"_s,
  "Aliquam"_s,
  "condimentum"_s,
  "rhoncus"_s,
  "facilisis"_s,
  "Nulla"_s,
  "euismod"_s,
  "ante"_s,
  "lorem"_s,
  "ut"_s,
  "laoreet"_s,
  "orci"_s,
  "auctor"_s,
  "ut"_s,
  "Cras"_s,
  "gravida"_s,
  "risus"_s,
  "ac"_s,
  "mauris"_s,
  "scelerisque"_s,
  "ut"_s,
  "pretium"_s,
  "ante"_s,
  "malesuada"_s,
  "Nulla"_s,
  "facilisis"_s,
  "molestie"_s,
  "elit"_s,
  "sed"_s,
  "pellentesque"_s,
  "Morbi"_s,
  "id"_s,
  "odio"_s,
  "nunc"_s,
  "Cras"_s,
  "fermentum"_s,
  "augue"_s,
  "a"_s,
  "felis"_s,
  "aliquet"_s,
  "placerat"_s,
  "Mauris"_s,
  "lorem"_s,
  "ex"_s,
  "fermentum"_s,
  "vel"_s,
  "euismod"_s,
  "consectetur"_s,
  "fermentum"_s,
  "eget"_s,
  "nulla"_s,
  "Mauris"_s,
  "quis"_s,
  "pulvinar"_s,
  "lorem"_s,
  "in"_s,
  "lorem"_s,
  "in"_s,
  "dapibus"_s,
  "lacus"_s,
  "Nunc"_s,
  "et"_s,
  "venenatis"_s,
  "justo"_s,
  "tincidunt"_s,
  "rhoncus"_s,
  "sem"_s,
  "Lorem"_s,
  "ipsum"_s,
  "dolor"_s,
  "sit"_s,
  "amet"_s,
  "consectetur"_s,
  "adipiscing"_s,
  "elit"_s,
  "Aenean"_s,
  "dictum"_s,
  "volutpat"_s,
  "leo"_s,
  "eu"_s,
  "commodo"_s,
  "nisl"_s,
  "eleifend"_s,
  "eleifend"_s,
  "Donec"_s,
  "id"_s,
  "elit"_s,
  "viverra"_s,
  "bibendum"_s,
  "mi"_s,
  "ut"_s,
  "tincidunt"_s,
  "dui"_s,
  "Mauris"_s,
  "vel"_s,
  "lacinia"_s,
  "est"_s,
  "Mauris"_s,
  "sit"_s,
  "amet"_s,
  "scelerisque"_s,
  "diam"_s,
  "non"_s,
  "fringilla"_s,
  "nisi"_s,
  "Cras"_s,
  "iaculis"_s,
  "neque"_s,
  "leo"_s,
  "pretium"_s,
  "pellentesque"_s,
  "est"_s,
  "eleifend"_s,
  "a"_s,
  "Integer"_s,
  "egestas"_s,
  "rutrum"_s,
  "semper"_s,
  "Mauris"_s,
  "vitae"_s,
  "iaculis"_s,
  "diam"_s,
  "placerat"_s,
  "luctus"_s,
  "arcu"_s,
  "Praesent"_s,
  "pellentesque"_s,
  "egestas"_s,
  "massa"_s,
  "sed"_s,
  "varius"_s,
  "Aliquam"_s,
  "sit"_s,
  "Aliquam"_s,
  "sit"_s,
  "amet"_s,
  "leo"_s,
  "ac"_s,
  "turpis"_s,
  "lobortis"_s,
  "pretium"_s,
  "id"_s,
  "vitae"_s,
  "tellus"_s,
  "Phasellus"_s,
  "ut"_s,
  "turpis"_s,
  "ac"_s,
  "libero"_s,
  "pretium"_s,
  "eleifend"_s,
  "vitae"_s,
  "sit"_s,
  "amet"_s,
  "tortor"_s,
  "Curabitur"_s,
  "dictum"_s,
  "id"_s,
  "nulla"_s,
  "non"_s,
  "vehicula"_s,
  "Quisque"_s,
  "nec"_s,
  "purus"_s,
  "vulputate"_s,
  "felis"_s,
  "Quisque"_s,
  "nec"_s,
  "purus"_s,
  "vulputate"_s,
  "felis"_s,
  "pellentesque"_s,
  "lacinia"_s,
  "Donec"_s,
  "vel"_s,
  "consectetur"_s,
  "lacus"_s,
  "Aenean"_s,
  "lectus"_s,
  "sapien"_s,
  "tincidunt"_s,
  "a"_s,
  "tortor"_s,
  "sed"_s,
  "fermentum"_s,
  "tristique"_s,
  "nibh"_s,
  "Ut"_s,
  "eget"_s,
  "justo"_s,
  "lorem"_s,
  "Morbi"_s,
  "efficitur"_s,
  "elementum"_s,
  "efficitur"_s,
  "Vestibulum"_s,
  "auctor"_s,
  "sem"_s,
  "vel"_s,
  "efficitur"_s,
  "auctor"_s,
  "Sed"_s,
  "diam"_s,
  "nisi"_s,
  "dignissim"_s,
  "vitae"_s,
  "lectus"_s,
  "sit"_s,
  "amet"_s,
  "lacinia"_s,
  "pulvinar"_s,
  "nisi"_s,
  "Pellentesque"_s,
  "malesuada"_s,
  "dolor"_s,
  "vitae"_s,
  "egestas"_s,
  "tempus"_s,
  "Mauris"_s,
  "placerat"_s,
  "ex"_s,
  "eu"_s,
  "est"_s,
  "posuere"_s,
  "dapibus"_s,
  "Pellentesque"_s,
  "bibendum"_s,
  "dui"_s,
  "nec"_s,
  "sodales"_s,
  "congue"_s,
  "lacus"_s,
  "nibh"_s,
  "pulvinar"_s,
  "diam"_s,
  "eu"_s,
  "tempus"_s,
  "sapien"_s,
  "eros"_s,
  "quis"_s,
  "arcu"_s,
  "Class"_s,
  "aptent"_s,
  "taciti"_s,
  "sociosqu"_s,
  "ad"_s,
  "Lorem ipsum dolor sit amet,"_s,
  " consectetur adipiscing elit."_s,
  "Lorconsectetur adipiscing elit."_s,
  "Lorem ipsum dolor sit amet"_s,
  "Nunc quis metus et dolor porttitor "_s,
  "ultricies vitae sit "_s,
  "amet ipsum. Nunc "
  "nec urna in urna vulputate molestie"_s,
  "sed vel augue. Cras egestas ut est"_s,
  " sodales elementum.,"_s,
  "sed vel augue. Cras egestas ut est"_s,
  "Pellentesque at nibh suscipit"_s,
  "Pellentesque at nibh suscipit"_s,
  "Pellentesque at nibh suscipit"_s,
  "nisi aliquet blandit non sed"_s,
  " quam. Vestibulum id"_s,
  "nisi aliquet blandit non sed quam. Vestibulum"_s,
  "sem a nibh "_s,
  "tristique gravida."_s,
  "sem a tristique gravida."_s,
  "Quisque turpis quam, tempor mollis lectus ut,"_s,
  "Quisque turpis quam, mollis "_s,
  "lectus ut,"_s,
  "egestas convallis quam. Duis et nulla quis dolor lobortis dapibus."_s,
  "egestas convallis quam. Duis et nulla quis dolor lobortis dapibus"_s,
  "egestas convallis quam. Duis et nulla"_s,
  " quis dolor lobortis dapibu"_s,
  "Donec nisl dui, volutpat nec ex at, consectetur auctor nisl."_s,
  "Fusce at ipsum consectetur,"_s,
  "tincidunt nunc eget, pharetra justo. Vivamus dapibus,"_s,
  "libero eget vehicula pellentesque, est elit tincidunt augue,"_s,
  "ac efficitur felis enim ac lectus. Duis massa massa,"_s,
  "ac efficitur felis enim ac lectus. Duis massa massa,"_s,
  "ac efficitur felis enim ac lectus. Duis"_s,
  " massa massa,"_s,
  " feugiat ac dolor vel, euismod blandit risus. Maecenas commodo massa "
  "est, non scelerisque libero rutrum ac."_s,
  " Nunc tincidunt eleifend odio sed consequat. In in lacinia nibh."_s,
  " Maecenas fringilla feugiat felis vitae eleifend. Mauris mi ligula, "
  "convallis eu viverra et,"_s,
  " gravida id mauris. Pellentesque fermentum"_s,
  " elit purus, at dapibus metus "
  "dapibus quis."_s,
  " Mauris eget quam a turpis euismod tincidunt. Suspendisse ultrices "
  "ullamcorper odio at interdum."_s,
  " Praesent ipsum leo, convallis sit amet "_s,
  "ligula lobortis, imperdiet "
  "volutpat dolor."_s,
  " Maecenas ut varius tellus,"_s,
  " vel ultricies massa. Vivamus tempor, magna eget porta tempus, turpis "
  "neque rutrum elit, eu "_s,
  "fermentum magna velit nec ligula."_s,
  "In ac magna ut dolor congue dictum sit "_s,
  "amet ut magna. Fusce cursus vehicula odio,"_s,
  " nec elementum metus dignissim "
  "in."_s,
  " Suspendisse potenti. Donec non massa vel massa pharetra lacinia."_s,
  " Fusce rhoncus felis nisi,"_s,
  " eu dictum quam laoreet vel. Donec a purus "
  "arcu."_s,
  " Sed maximus sollicitudin dolor sit amet ultrices. Mauris tortor nunc, ",
  "tincidunt sed varius iaculis, consequat ut nibh. "_s,
  "tincidunt sed varius iaculis, nibh. "_s,
  "Duis nec lobortis sapien. Pellentesque"_s,
  " elementum congue libero, et "
  "congue ligula varius at."_s,
  " Duis elit odio, ultricies nec arcu in,"_s,
  " Duis elit odio, ultricies nec arcu in,"_s,
  " faucibus mattis libero. Integer "_s,
  "bibendum eros in lectus ornare aliquet.",
  " Vestibulum ac justo metus. Nullam sit amet tortor semper,"_s,
  " fringilla urna vel, fringilla erat. Vestibulum"_s,
  " malesuada libero quis ipsum scelerisque,"_s,
  " placerat pellentesque tortor "
  "facilisis. Nulla facilisi."_s,
  " Sed pharetra velit in rhoncus vulputate. Lorem ipsum dolor sit amet, ",
  "consectetur adipiscing elit."_s,
  " Aliquam vitae placerat ipsum. Cras nec mi "
  "non odio mattis commodo. "_s,
  "Nulla aliquam odio tortor, at faucibus ligula "
  "malesuada at."_s,
  " Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
  "posuere cubilia curae; Morbi eget finibus libero."_s,
  "Mauris at ante nulla."_s,
  " Fusce felis arcu, vehicula molestie commodo ac,"_s,
  " "
  "aliquam eu sem. Sed congue consequat arcu"_s,
  " at vehicula. Vivamus porta "
  "rutrum mi,"_s,
  " ac sollicitudin nulla suscipit ac."_s,
  " Vestibulum quis magna lorem. "
  "Pellentesque habitant morbi tristique"_s,
  " senectus et netus et malesuada "
  "fames ac turpis egestas."_s,
  " Vestibulum auctor, massa non"_s,
  " hendrerit bibendum, lorem ante laoreet mi, "
  "quis lobortis ipsum odio a velit."_s,
  " Vestibulum auctor, massa non hendrerit bibendum, lorem ante laoreet mi, "
  "quis lobortis ipsum odio a velit."_s,
  " Suspendisse eget aliquam orci."_s};

static void BM_Map_Probe32(benchmark::State & state)
{
  StrDict<i64, u32> map;
  i64 const        num_inserts = state.range(0);
  i64              num_queries = 0;

  for (auto _ : state)
  {
    for (i64 i = 0; i < num_inserts; i++)
    {
      auto & dp = DATASET[i % size(DATASET)];
      map.push(dp, 0).unwrap();
    }
    for (auto & dp : DATASET)
    {
      benchmark::DoNotOptimize(map.has(dp));
      num_queries++;
    }
    for (auto & dp : DATASET)
    {
      map.erase(dp);
    }
  }

  state.SetItemsProcessed(num_inserts);
  state.counters["num_queries"] =
    benchmark::Counter{(f64) num_queries, benchmark::Counter::kIsRate};
}

static void BM_Map_Probe64(benchmark::State & state)
{
  StrDict<i64, u64> map;
  i64 const        num_inserts = state.range(0);
  i64              num_queries = 0;

  for (auto _ : state)
  {
    for (i64 i = 0; i < num_inserts; i++)
    {
      auto & dp = DATASET[i % size(DATASET)];
      map.push(dp, 0).unwrap();
    }
    for (auto & dp : DATASET)
    {
      benchmark::DoNotOptimize(map.has(dp));
      num_queries++;
    }
    for (auto & dp : DATASET)
    {
      map.erase(dp);
    }
  }

  state.SetItemsProcessed(num_inserts);
  state.counters["num_queries"] =
    benchmark::Counter{(f64) num_queries, benchmark::Counter::kIsRate};
}

template <typename T>
struct std_allocator
{
  using size_type       = size_t;
  using difference_type = ptrdiff_t;
  using pointer         = T *;
  using const_pointer   = T const *;
  using reference       = T &;
  using const_reference = T const &;
  using value_type      = T;

  template <typename U>
  struct rebind
  {
    typedef std_allocator<U> other;
  };

  std_allocator() throw()
  {
  }

  std_allocator(std_allocator const &) throw()
  {
  }

  template <typename U>
  std_allocator(std_allocator<U> const &) throw()
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
    if (!default_allocator->nalloc(s, temp))
    {
      throw std::bad_alloc();
    }
    return temp;
  }

  void deallocate(pointer p, size_type s)
  {
    default_allocator->ndealloc(s,p);
  }

  size_type max_size() const throw()
  {
    return std::numeric_limits<size_t>::max() / sizeof(T);
  }

  void construct(pointer p, T const & val)
  {
    new ((void *) p) T(val);
  }

  void destroy(pointer p)
  {
    p->~T();
  }
};

static void BM_StdMap_AshHasher(benchmark::State & state)
{
  std::unordered_map<Str, i64, SpanHash, StrEq,
                     std_allocator<std::pair<Str const, i64>>>
            map;
  auto num_inserts = state.range(0);
  i64       num_queries = 0;

  for (auto _ : state)
  {
    for (i64 i = 0; i < num_inserts; i++)
    {
      auto & dp = DATASET[i % size(DATASET)];
      map.emplace(dp, 0);
    }
    for (auto & dp : DATASET)
    {
      benchmark::DoNotOptimize(map.contains(dp));
      num_queries++;
    }
    for (auto & dp : DATASET)
    {
      map.erase(dp);
    }
  }

  state.SetItemsProcessed(num_inserts);
  state.counters["num_queries"] =
    benchmark::Counter{(f64) num_queries, benchmark::Counter::kIsRate};
}

template <>
struct std::less<Str>
{
  bool operator()(Str a, Str b) const
  {
    std::string_view a_str{a.data(), a.size()};
    std::string_view b_str{b.data(), b.size()};
    return a_str < b_str;
  }
};

template <>
struct std::hash<Str>
{
  std::hash<std::string_view> hash;

  size_t operator()(Str str) const
  {
    return hash(std::string_view{str.data(), str.size()});
  }
};

static void BM_StdMapDefaultHash(benchmark::State & state)
{
  std::unordered_map<Str, i64, std::hash<Str>, StrEq,
                     std_allocator<std::pair<Str const, i64>>>
            map;
  auto num_inserts = state.range(0);
  i64       num_queries = 0;

  for (auto _ : state)
  {
    for (i64 i = 0; i < num_inserts; i++)
    {
      auto & dp = DATASET[i % size(DATASET)];
      map.emplace(dp, 0);
    }
    for (auto & dp : DATASET)
    {
      benchmark::DoNotOptimize(map.contains(dp));
      num_queries++;
    }
    for (auto & dp : DATASET)
    {
      map.erase(dp);
    }
  }

  state.SetItemsProcessed(num_inserts);
  state.counters["num_queries"] =
    benchmark::Counter{(f64) num_queries, benchmark::Counter::kIsRate};
}

static void BM_StdMapDefaultHashDefaultAlloc(benchmark::State & state)
{
  std::unordered_map<Str, i64, std::hash<Str>, StrEq>
            map;
 auto num_inserts = state.range(0);
  i64       num_queries = 0;

  for (auto _ : state)
  {
    for (i64 i = 0; i < num_inserts; i++)
    {
      auto & dp = DATASET[i % size(DATASET)];
      map.emplace(dp, 0);
    }
    for (auto & dp : DATASET)
    {
      benchmark::DoNotOptimize(map.contains(dp));
      num_queries++;
    }
    for (auto & dp : DATASET)
    {
      map.erase(dp);
    }
  }

  state.SetItemsProcessed(num_inserts);
  state.counters["num_queries"] =
    benchmark::Counter{(f64) num_queries, benchmark::Counter::kIsRate};
}

void BM_StdOrderedMapDefaultAlloc(benchmark::State & state)
{
  std::map<Str, i64> map;
  i64 const                       num_inserts = state.range(0);
  i64                             num_queries = 0;

  for (auto _ : state)
  {
    for (i64 i = 0; i < num_inserts; i++)
    {
      auto & dp = DATASET[i % size(DATASET)];
      map.emplace(dp, 0);
    }
    for (auto & dp : DATASET)
    {
      benchmark::DoNotOptimize(map.contains(dp));
      num_queries++;
    }
    for (auto & dp : DATASET)
    {
      map.erase(dp);
    }
  }

  state.SetItemsProcessed(num_inserts);
  state.counters["num_queries"] =
    benchmark::Counter{(f64) num_queries, benchmark::Counter::kIsRate};
}

#define ADD_BENCH(name) \
  BENCHMARK(BM_##name)->Name(#name)->Arg(std::size(DATASET) * 4)

ADD_BENCH(Map_Probe32);
ADD_BENCH(Map_Probe64);
ADD_BENCH(StdMap_AshHasher);
ADD_BENCH(StdMapDefaultHash);
ADD_BENCH(StdMapDefaultHashDefaultAlloc);

BENCHMARK_MAIN();
