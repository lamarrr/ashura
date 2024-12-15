/// SPDX-License-Identifier: MIT
#include "ashura/std/map.h"
#include "ashura/std/types.h"
#include "stdint.h"
#include <algorithm>
#include <benchmark/benchmark.h>
#include <map>
#include <unordered_map>

using namespace ash;

constexpr Span<char const> DATASET[] = {
    "Lorem"_str,
    "ipsum"_str,
    "dolor"_str,
    "sit"_str,
    "amet"_str,
    "consectetur"_str,
    "adipiscing"_str,
    "elit"_str,
    "Nullam"_str,
    "ultricies"_str,
    "purus"_str,
    "facilisis"_str,
    "orci"_str,
    "euismod"_str,
    "eleifend"_str,
    "Pellentesque"_str,
    "bibendum"_str,
    "pretium"_str,
    "quam"_str,
    "et"_str,
    "gravida"_str,
    "Proin"_str,
    "tortor"_str,
    "urna"_str,
    "convallis"_str,
    "eget"_str,
    "neque"_str,
    "sed"_str,
    "commodo"_str,
    "consectetur"_str,
    "magna"_str,
    "Praesent"_str,
    "ac"_str,
    "nisl"_str,
    "eu"_str,
    "purus"_str,
    "pretium"_str,
    "ultrices"_str,
    "vitae"_str,
    "ut"_str,
    "ante"_str,
    "Etiam"_str,
    "sit"_str,
    "amet"_str,
    "sapien"_str,
    "elit"_str,
    "Morbi"_str,
    "bibendum"_str,
    "consectetur"_str,
    "dolor"_str,
    "convallis"_str,
    "egestas"_str,
    "Interdum"_str,
    "et"_str,
    "malesuada"_str,
    "fames"_str,
    "ac"_str,
    "ante"_str,
    "ipsum"_str,
    "primis"_str,
    "in"_str,
    "faucibus"_str,
    "Phasellus"_str,
    "condimentum"_str,
    "hendrerit"_str,
    "tellus"_str,
    "Nam"_str,
    "eleifend"_str,
    "justo"_str,
    "at"_str,
    "ultrices"_str,
    "sodales"_str,
    "Aliquam"_str,
    "sit"_str,
    "amet"_str,
    "ante"_str,
    "in"_str,
    "ligula"_str,
    "elementum"_str,
    "dignissim"_str,
    "Integer"_str,
    "in"_str,
    "justo"_str,
    "in"_str,
    "ipsum"_str,
    "varius"_str,
    "imperdiet"_str,
    "a"_str,
    "sed"_str,
    "dui"_str,
    "Class"_str,
    "aptent"_str,
    "taciti"_str,
    "sociosqu"_str,
    "ad"_str,
    "litora"_str,
    "torquent"_str,
    "per"_str,
    "conubia"_str,
    "nostra"_str,
    "per"_str,
    "inceptos"_str,
    "himenaeos"_str,
    "Nunc"_str,
    "leo"_str,
    "eros"_str,
    "ullamcorper"_str,
    "vitae"_str,
    "fermentum"_str,
    "id"_str,
    "molestie"_str,
    "sit"_str,
    "amet"_str,
    "tellus"_str,
    "Sed"_str,
    "vel"_str,
    "nunc"_str,
    "sed"_str,
    "nulla"_str,
    "varius"_str,
    "porttitor"_str,
    "Vivamus"_str,
    "vitae"_str,
    "molestie"_str,
    "sapien"_str,
    "Sed"_str,
    "imperdiet"_str,
    "justo"_str,
    "mauris"_str,
    "sit"_str,
    "amet"_str,
    "elementum"_str,
    "metus"_str,
    "tempor"_str,
    "ac"_str,
    "Praesent"_str,
    "et"_str,
    "diam"_str,
    "et"_str,
    "orci"_str,
    "blandit"_str,
    "convallis"_str,
    "rutrum"_str,
    "eu"_str,
    "nisl"_str,
    "Donec"_str,
    "vulputate"_str,
    "hendrerit"_str,
    "nisi"_str,
    "sit"_str,
    "amet"_str,
    "rutrum"_str,
    "Nullam"_str,
    "faucibus"_str,
    "tincidunt"_str,
    "lectus"_str,
    "eu"_str,
    "gravida"_str,
    "est"_str,
    "faucibus"_str,
    "imperdiet"_str,
    "Nam"_str,
    "varius"_str,
    "vehicula"_str,
    "risus"_str,
    "ut"_str,
    "tempus"_str,
    "Aliquam"_str,
    "erat"_str,
    "volutpat"_str,
    "In"_str,
    "pellentesque"_str,
    "auctor"_str,
    "vulputate"_str,
    "Suspendisse"_str,
    "rhoncus"_str,
    "magna"_str,
    "quis"_str,
    "tincidunt"_str,
    "gravida"_str,
    "libero"_str,
    "ex"_str,
    "egestas"_str,
    "diam"_str,
    "eget"_str,
    "hendrerit"_str,
    "odio"_str,
    "ante"_str,
    "eget"_str,
    "velit"_str,
    "Proin"_str,
    "quis"_str,
    "nulla"_str,
    "placerat"_str,
    "sagittis"_str,
    "augue"_str,
    "in"_str,
    "mattis"_str,
    "tortor"_str,
    "Aliquam"_str,
    "accumsan"_str,
    "metus"_str,
    "eu"_str,
    "nisl"_str,
    "hendrerit"_str,
    "non"_str,
    "hendrerit"_str,
    "justo"_str,
    "commodo"_str,
    "Suspendisse"_str,
    "bibendum"_str,
    "euismod"_str,
    "gravida"_str,
    "Ut"_str,
    "nisi"_str,
    "libero"_str,
    "facilisis"_str,
    "nec"_str,
    "erat"_str,
    "a"_str,
    "tempus"_str,
    "ullamcorper"_str,
    "risus"_str,
    "Curabitur"_str,
    "tortor"_str,
    "mi"_str,
    "suscipit"_str,
    "sit"_str,
    "amet"_str,
    "odio"_str,
    "quis"_str,
    "egestas"_str,
    "suscipit"_str,
    "elit"_str,
    "Nam"_str,
    "id"_str,
    "velit"_str,
    "vel"_str,
    "eros"_str,
    "pharetra"_str,
    "tristique"_str,
    "nec"_str,
    "at"_str,
    "orci"_str,
    "Lorem"_str,
    "ipsum"_str,
    "dolor"_str,
    "sit"_str,
    "amet"_str,
    "consectetur"_str,
    "adipiscing"_str,
    "elit"_str,
    "Nulla"_str,
    "dignissim"_str,
    "magna"_str,
    "quis"_str,
    "venenatis"_str,
    "ornare"_str,
    "libero"_str,
    "turpis"_str,
    "sagittis"_str,
    "tortor"_str,
    "ut"_str,
    "posuere"_str,
    "dui"_str,
    "nunc"_str,
    "eu"_str,
    "erat"_str,
    "Aenean"_str,
    "ullamcorper"_str,
    "interdum"_str,
    "mi"_str,
    "eu"_str,
    "auctor"_str,
    "Pellentesque"_str,
    "tempor"_str,
    "elit"_str,
    "vitae"_str,
    "urna"_str,
    "consectetur"_str,
    "eu"_str,
    "imperdiet"_str,
    "orci"_str,
    "iaculis"_str,
    "In"_str,
    "eu"_str,
    "fringilla"_str,
    "augue"_str,
    "Curabitur"_str,
    "maximus"_str,
    "nunc"_str,
    "orci"_str,
    "a"_str,
    "elementum"_str,
    "ipsum"_str,
    "blandit"_str,
    "in"_str,
    "Aliquam"_str,
    "erat"_str,
    "volutpat"_str,
    "Donec"_str,
    "feugiat"_str,
    "ipsum"_str,
    "non"_str,
    "scelerisque"_str,
    "euismod"_str,
    "Suspendisse"_str,
    "sem"_str,
    "diam"_str,
    "consequat"_str,
    "at"_str,
    "tempus"_str,
    "eu"_str,
    "lacinia"_str,
    "et"_str,
    "lectus"_str,
    "Quisque"_str,
    "ac"_str,
    "urna"_str,
    "consectetur"_str,
    "vestibulum"_str,
    "nisl"_str,
    "at"_str,
    "venenatis"_str,
    "dui"_str,
    "Donec"_str,
    "maximus"_str,
    "aliquam"_str,
    "ornare"_str,
    "Pellentesque"_str,
    "sit"_str,
    "amet"_str,
    "orci"_str,
    "non"_str,
    "ligula"_str,
    "hendrerit"_str,
    "consectetur"_str,
    "Vestibulum"_str,
    "varius"_str,
    "eros"_str,
    "odio"_str,
    "consequat"_str,
    "aliquam"_str,
    "lacus"_str,
    "interdum"_str,
    "condimentum"_str,
    "Curabitur"_str,
    "blandit"_str,
    "ut"_str,
    "ante"_str,
    "et"_str,
    "varius"_str,
    "Pellentesque"_str,
    "lobortis"_str,
    "nibh"_str,
    "eu"_str,
    "consequat"_str,
    "feugiat"_str,
    "Phasellus"_str,
    "ultricies"_str,
    "enim"_str,
    "purus"_str,
    "et"_str,
    "ultricies"_str,
    "nulla"_str,
    "molestie"_str,
    "ac"_str,
    "Pellentesque"_str,
    "lacus"_str,
    "urna"_str,
    "tristique"_str,
    "a"_str,
    "urna"_str,
    "sed"_str,
    "fringilla"_str,
    "consequat"_str,
    "orci"_str,
    "Nulla"_str,
    "pharetra"_str,
    "commodo"_str,
    "ipsum"_str,
    "vel"_str,
    "volutpat"_str,
    "tellus"_str,
    "porttitor"_str,
    "a"_str,
    "Praesent"_str,
    "id"_str,
    "augue"_str,
    "lacus"_str,
    "Mauris"_str,
    "sed"_str,
    "aliquet"_str,
    "dui"_str,
    "Vestibulum"_str,
    "fringilla"_str,
    "lacus"_str,
    "elit"_str,
    "ac"_str,
    "tincidunt"_str,
    "justo"_str,
    "convallis"_str,
    "eget"_str,
    "Aenean"_str,
    "vel"_str,
    "lacinia"_str,
    "mauris"_str,
    "eu"_str,
    "fringilla"_str,
    "lacus"_str,
    "Nulla"_str,
    "pulvinar"_str,
    "dolor"_str,
    "quis"_str,
    "dui"_str,
    "aliquet"_str,
    "elementum"_str,
    "Interdum"_str,
    "et"_str,
    "malesuada"_str,
    "fames"_str,
    "ac"_str,
    "ante"_str,
    "ipsum"_str,
    "primis"_str,
    "in"_str,
    "faucibus"_str,
    "Curabitur"_str,
    "eu"_str,
    "ex"_str,
    "libero"_str,
    "In"_str,
    "hac"_str,
    "habitasse"_str,
    "platea"_str,
    "dictumst"_str,
    "Donec"_str,
    "tincidunt"_str,
    "bibendum"_str,
    "urna"_str,
    "sit"_str,
    "amet"_str,
    "suscipit"_str,
    "Quisque"_str,
    "at"_str,
    "purus"_str,
    "nec"_str,
    "tellus"_str,
    "sagittis"_str,
    "fringilla"_str,
    "In"_str,
    "hac"_str,
    "habitasse"_str,
    "platea"_str,
    "dictumst"_str,
    "Curabitur"_str,
    "eget"_str,
    "justo"_str,
    "et"_str,
    "ante"_str,
    "dapibus"_str,
    "sagittis"_str,
    "In"_str,
    "tempor"_str,
    "nisi"_str,
    "in"_str,
    "cursus"_str,
    "tincidunt"_str,
    "nisl"_str,
    "ipsum"_str,
    "accumsan"_str,
    "libero"_str,
    "eu"_str,
    "imperdiet"_str,
    "arcu"_str,
    "massa"_str,
    "eu"_str,
    "nisl"_str,
    "In"_str,
    "et"_str,
    "vulputate"_str,
    "ante"_str,
    "eu"_str,
    "ullamcorper"_str,
    "justo"_str,
    "Fusce"_str,
    "quis"_str,
    "augue"_str,
    "eu"_str,
    "nunc"_str,
    "feugiat"_str,
    "ultrices"_str,
    "Nulla"_str,
    "est"_str,
    "augue"_str,
    "pretium"_str,
    "et"_str,
    "volutpat"_str,
    "ac"_str,
    "mattis"_str,
    "in"_str,
    "ante"_str,
    "Aenean"_str,
    "molestie"_str,
    "magna"_str,
    "lacus"_str,
    "Ut"_str,
    "lorem"_str,
    "sapien"_str,
    "placerat"_str,
    "sit"_str,
    "amet"_str,
    "porta"_str,
    "et"_str,
    "congue"_str,
    "blandit"_str,
    "neque"_str,
    "Mauris"_str,
    "tristique"_str,
    "ipsum"_str,
    "a"_str,
    "ullamcorper"_str,
    "dignissim"_str,
    "sapien"_str,
    "nisi"_str,
    "consectetur"_str,
    "diam"_str,
    "vitae"_str,
    "tincidunt"_str,
    "nisl"_str,
    "est"_str,
    "id"_str,
    "enim"_str,
    "Quisque"_str,
    "ut"_str,
    "nulla"_str,
    "velit"_str,
    "Vestibulum"_str,
    "sit"_str,
    "amet"_str,
    "libero"_str,
    "turpis"_str,
    "Phasellus"_str,
    "tristique"_str,
    "justo"_str,
    "non"_str,
    "semper"_str,
    "laoreet"_str,
    "Maecenas"_str,
    "vehicula"_str,
    "congue"_str,
    "ante"_str,
    "sed"_str,
    "ultrices"_str,
    "Sed"_str,
    "ex"_str,
    "elit"_str,
    "scelerisque"_str,
    "non"_str,
    "ligula"_str,
    "et"_str,
    "vestibulum"_str,
    "tristique"_str,
    "est"_str,
    "In"_str,
    "egestas"_str,
    "porttitor"_str,
    "tortor"_str,
    "eget"_str,
    "tempor"_str,
    "enim"_str,
    "dictum"_str,
    "non"_str,
    "Vestibulum"_str,
    "tincidunt"_str,
    "leo"_str,
    "sed"_str,
    "consequat"_str,
    "pulvinar"_str,
    "Morbi"_str,
    "dictum"_str,
    "mi"_str,
    "sit"_str,
    "amet"_str,
    "bibendum"_str,
    "ullamcorper"_str,
    "Integer"_str,
    "nunc"_str,
    "ipsum"_str,
    "varius"_str,
    "sit"_str,
    "amet"_str,
    "libero"_str,
    "sit"_str,
    "amet"_str,
    "viverra"_str,
    "commodo"_str,
    "ligula"_str,
    "Pellentesque"_str,
    "egestas"_str,
    "scelerisque"_str,
    "orci"_str,
    "id"_str,
    "interdum"_str,
    "elit"_str,
    "tristique"_str,
    "et"_str,
    "Fusce"_str,
    "non"_str,
    "leo"_str,
    "justo"_str,
    "Sed"_str,
    "enim"_str,
    "dui"_str,
    "malesuada"_str,
    "sed"_str,
    "eros"_str,
    "non"_str,
    "tristique"_str,
    "dictum"_str,
    "metus"_str,
    "In"_str,
    "non"_str,
    "lectus"_str,
    "feugiat"_str,
    "pulvinar"_str,
    "elit"_str,
    "vitae"_str,
    "interdum"_str,
    "ligula"_str,
    "Quisque"_str,
    "ac"_str,
    "justo"_str,
    "accumsan"_str,
    "aliquet"_str,
    "risus"_str,
    "id"_str,
    "dictum"_str,
    "nibh"_str,
    "Orci"_str,
    "varius"_str,
    "natoque"_str,
    "penatibus"_str,
    "et"_str,
    "magnis"_str,
    "dis"_str,
    "parturient"_str,
    "montes"_str,
    "nascetur"_str,
    "ridiculus"_str,
    "mus"_str,
    "Vestibulum"_str,
    "sodales"_str,
    "lacus"_str,
    "non"_str,
    "luctus"_str,
    "egestas"_str,
    "Ut"_str,
    "sed"_str,
    "consectetur"_str,
    "neque"_str,
    "Nulla"_str,
    "nec"_str,
    "arcu"_str,
    "interdum"_str,
    "auctor"_str,
    "nisi"_str,
    "non"_str,
    "posuere"_str,
    "dui"_str,
    "Aliquam"_str,
    "sodales"_str,
    "lacus"_str,
    "eget"_str,
    "diam"_str,
    "gravida"_str,
    "porta"_str,
    "Donec"_str,
    "iaculis"_str,
    "massa"_str,
    "ac"_str,
    "nulla"_str,
    "ultrices"_str,
    "id"_str,
    "pretium"_str,
    "ante"_str,
    "sodales"_str,
    "Curabitur"_str,
    "dignissim"_str,
    "purus"_str,
    "ex"_str,
    "ac"_str,
    "finibus"_str,
    "nisi"_str,
    "volutpat"_str,
    "eu"_str,
    "Suspendisse"_str,
    "eu"_str,
    "nibh"_str,
    "non"_str,
    "odio"_str,
    "varius"_str,
    "porttitor"_str,
    "Nulla"_str,
    "elementum"_str,
    "ullamcorper"_str,
    "ultrices"_str,
    "Sed"_str,
    "mattis"_str,
    "purus"_str,
    "libero"_str,
    "non"_str,
    "condimentum"_str,
    "leo"_str,
    "dapibus"_str,
    "tristique"_str,
    "In"_str,
    "malesuada"_str,
    "eleifend"_str,
    "tortor"_str,
    "non"_str,
    "ornare"_str,
    "enim"_str,
    "fringilla"_str,
    "eget"_str,
    "Sed"_str,
    "dolor"_str,
    "leo"_str,
    "commodo"_str,
    "sit"_str,
    "amet"_str,
    "felis"_str,
    "viverra"_str,
    "dapibus"_str,
    "convallis"_str,
    "tellus"_str,
    "Phasellus"_str,
    "rutrum"_str,
    "volutpat"_str,
    "leo"_str,
    "ut"_str,
    "imperdiet"_str,
    "neque"_str,
    "Cras"_str,
    "libero"_str,
    "orci"_str,
    "feugiat"_str,
    "vitae"_str,
    "enim"_str,
    "a"_str,
    "rutrum"_str,
    "egestas"_str,
    "felis"_str,
    "Curabitur"_str,
    "vel"_str,
    "ipsum"_str,
    "eget"_str,
    "eros"_str,
    "vestibulum"_str,
    "tempor"_str,
    "sit"_str,
    "amet"_str,
    "quis"_str,
    "purus"_str,
    "Aliquam"_str,
    "condimentum"_str,
    "rhoncus"_str,
    "facilisis"_str,
    "Nulla"_str,
    "euismod"_str,
    "ante"_str,
    "lorem"_str,
    "ut"_str,
    "laoreet"_str,
    "orci"_str,
    "auctor"_str,
    "ut"_str,
    "Cras"_str,
    "gravida"_str,
    "risus"_str,
    "ac"_str,
    "mauris"_str,
    "scelerisque"_str,
    "ut"_str,
    "pretium"_str,
    "ante"_str,
    "malesuada"_str,
    "Nulla"_str,
    "facilisis"_str,
    "molestie"_str,
    "elit"_str,
    "sed"_str,
    "pellentesque"_str,
    "Morbi"_str,
    "id"_str,
    "odio"_str,
    "nunc"_str,
    "Cras"_str,
    "fermentum"_str,
    "augue"_str,
    "a"_str,
    "felis"_str,
    "aliquet"_str,
    "placerat"_str,
    "Mauris"_str,
    "lorem"_str,
    "ex"_str,
    "fermentum"_str,
    "vel"_str,
    "euismod"_str,
    "consectetur"_str,
    "fermentum"_str,
    "eget"_str,
    "nulla"_str,
    "Mauris"_str,
    "quis"_str,
    "pulvinar"_str,
    "lorem"_str,
    "in"_str,
    "lorem"_str,
    "in"_str,
    "dapibus"_str,
    "lacus"_str,
    "Nunc"_str,
    "et"_str,
    "venenatis"_str,
    "justo"_str,
    "tincidunt"_str,
    "rhoncus"_str,
    "sem"_str,
    "Lorem"_str,
    "ipsum"_str,
    "dolor"_str,
    "sit"_str,
    "amet"_str,
    "consectetur"_str,
    "adipiscing"_str,
    "elit"_str,
    "Aenean"_str,
    "dictum"_str,
    "volutpat"_str,
    "leo"_str,
    "eu"_str,
    "commodo"_str,
    "nisl"_str,
    "eleifend"_str,
    "eleifend"_str,
    "Donec"_str,
    "id"_str,
    "elit"_str,
    "viverra"_str,
    "bibendum"_str,
    "mi"_str,
    "ut"_str,
    "tincidunt"_str,
    "dui"_str,
    "Mauris"_str,
    "vel"_str,
    "lacinia"_str,
    "est"_str,
    "Mauris"_str,
    "sit"_str,
    "amet"_str,
    "scelerisque"_str,
    "diam"_str,
    "non"_str,
    "fringilla"_str,
    "nisi"_str,
    "Cras"_str,
    "iaculis"_str,
    "neque"_str,
    "leo"_str,
    "pretium"_str,
    "pellentesque"_str,
    "est"_str,
    "eleifend"_str,
    "a"_str,
    "Integer"_str,
    "egestas"_str,
    "rutrum"_str,
    "semper"_str,
    "Mauris"_str,
    "vitae"_str,
    "iaculis"_str,
    "diam"_str,
    "placerat"_str,
    "luctus"_str,
    "arcu"_str,
    "Praesent"_str,
    "pellentesque"_str,
    "egestas"_str,
    "massa"_str,
    "sed"_str,
    "varius"_str,
    "Aliquam"_str,
    "sit"_str,
    "Aliquam"_str,
    "sit"_str,
    "amet"_str,
    "leo"_str,
    "ac"_str,
    "turpis"_str,
    "lobortis"_str,
    "pretium"_str,
    "id"_str,
    "vitae"_str,
    "tellus"_str,
    "Phasellus"_str,
    "ut"_str,
    "turpis"_str,
    "ac"_str,
    "libero"_str,
    "pretium"_str,
    "eleifend"_str,
    "vitae"_str,
    "sit"_str,
    "amet"_str,
    "tortor"_str,
    "Curabitur"_str,
    "dictum"_str,
    "id"_str,
    "nulla"_str,
    "non"_str,
    "vehicula"_str,
    "Quisque"_str,
    "nec"_str,
    "purus"_str,
    "vulputate"_str,
    "felis"_str,
    "Quisque"_str,
    "nec"_str,
    "purus"_str,
    "vulputate"_str,
    "felis"_str,
    "pellentesque"_str,
    "lacinia"_str,
    "Donec"_str,
    "vel"_str,
    "consectetur"_str,
    "lacus"_str,
    "Aenean"_str,
    "lectus"_str,
    "sapien"_str,
    "tincidunt"_str,
    "a"_str,
    "tortor"_str,
    "sed"_str,
    "fermentum"_str,
    "tristique"_str,
    "nibh"_str,
    "Ut"_str,
    "eget"_str,
    "justo"_str,
    "lorem"_str,
    "Morbi"_str,
    "efficitur"_str,
    "elementum"_str,
    "efficitur"_str,
    "Vestibulum"_str,
    "auctor"_str,
    "sem"_str,
    "vel"_str,
    "efficitur"_str,
    "auctor"_str,
    "Sed"_str,
    "diam"_str,
    "nisi"_str,
    "dignissim"_str,
    "vitae"_str,
    "lectus"_str,
    "sit"_str,
    "amet"_str,
    "lacinia"_str,
    "pulvinar"_str,
    "nisi"_str,
    "Pellentesque"_str,
    "malesuada"_str,
    "dolor"_str,
    "vitae"_str,
    "egestas"_str,
    "tempus"_str,
    "Mauris"_str,
    "placerat"_str,
    "ex"_str,
    "eu"_str,
    "est"_str,
    "posuere"_str,
    "dapibus"_str,
    "Pellentesque"_str,
    "bibendum"_str,
    "dui"_str,
    "nec"_str,
    "sodales"_str,
    "congue"_str,
    "lacus"_str,
    "nibh"_str,
    "pulvinar"_str,
    "diam"_str,
    "eu"_str,
    "tempus"_str,
    "sapien"_str,
    "eros"_str,
    "quis"_str,
    "arcu"_str,
    "Class"_str,
    "aptent"_str,
    "taciti"_str,
    "sociosqu"_str,
    "ad"_str,
    "Lorem ipsum dolor sit amet,"_str,
    " consectetur adipiscing elit."_str,
    "Lorconsectetur adipiscing elit."_str,
    "Lorem ipsum dolor sit amet"_str,
    "Nunc quis metus et dolor porttitor "_str,
    "ultricies vitae sit "_str,
    "amet ipsum. Nunc "
    "nec urna in urna vulputate molestie"_str,
    "sed vel augue. Cras egestas ut est"_str,
    " sodales elementum.,"_str,
    "sed vel augue. Cras egestas ut est"_str,
    "Pellentesque at nibh suscipit"_str,
    "Pellentesque at nibh suscipit"_str,
    "Pellentesque at nibh suscipit"_str,
    "nisi aliquet blandit non sed"_str,
    " quam. Vestibulum id"_str,
    "nisi aliquet blandit non sed quam. Vestibulum"_str,
    "sem a nibh "_str,
    "tristique gravida."_str,
    "sem a tristique gravida."_str,
    "Quisque turpis quam, tempor mollis lectus ut,"_str,
    "Quisque turpis quam, mollis "_str,
    "lectus ut,"_str,
    "egestas convallis quam. Duis et nulla quis dolor lobortis dapibus."_str,
    "egestas convallis quam. Duis et nulla quis dolor lobortis dapibus"_str,
    "egestas convallis quam. Duis et nulla"_str,
    " quis dolor lobortis dapibu"_str,
    "Donec nisl dui, volutpat nec ex at, consectetur auctor nisl."_str,
    "Fusce at ipsum consectetur,"_str,
    "tincidunt nunc eget, pharetra justo. Vivamus dapibus,"_str,
    "libero eget vehicula pellentesque, est elit tincidunt augue,"_str,
    "ac efficitur felis enim ac lectus. Duis massa massa,"_str,
    "ac efficitur felis enim ac lectus. Duis massa massa,"_str,
    "ac efficitur felis enim ac lectus. Duis"_str,
    " massa massa,"_str,
    " feugiat ac dolor vel, euismod blandit risus. Maecenas commodo massa "
    "est, non scelerisque libero rutrum ac."_str,
    " Nunc tincidunt eleifend odio sed consequat. In in lacinia nibh."_str,
    " Maecenas fringilla feugiat felis vitae eleifend. Mauris mi ligula, "
    "convallis eu viverra et,"_str,
    " gravida id mauris. Pellentesque fermentum"_str,
    " elit purus, at dapibus metus "
    "dapibus quis."_str,
    " Mauris eget quam a turpis euismod tincidunt. Suspendisse ultrices "
    "ullamcorper odio at interdum."_str,
    " Praesent ipsum leo, convallis sit amet "_str,
    "ligula lobortis, imperdiet "
    "volutpat dolor."_str,
    " Maecenas ut varius tellus,"_str,
    " vel ultricies massa. Vivamus tempor, magna eget porta tempus, turpis "
    "neque rutrum elit, eu "_str,
    "fermentum magna velit nec ligula."_str,
    "In ac magna ut dolor congue dictum sit "_str,
    "amet ut magna. Fusce cursus vehicula odio,"_str,
    " nec elementum metus dignissim "
    "in."_str,
    " Suspendisse potenti. Donec non massa vel massa pharetra lacinia."_str,
    " Fusce rhoncus felis nisi,"_str,
    " eu dictum quam laoreet vel. Donec a purus "
    "arcu."_str,
    " Sed maximus sollicitudin dolor sit amet ultrices. Mauris tortor nunc, ",
    "tincidunt sed varius iaculis, consequat ut nibh. "_str,
    "tincidunt sed varius iaculis, nibh. "_str,
    "Duis nec lobortis sapien. Pellentesque"_str,
    " elementum congue libero, et "
    "congue ligula varius at."_str,
    " Duis elit odio, ultricies nec arcu in,"_str,
    " Duis elit odio, ultricies nec arcu in,"_str,
    " faucibus mattis libero. Integer "_str,
    "bibendum eros in lectus ornare aliquet.",
    " Vestibulum ac justo metus. Nullam sit amet tortor semper,"_str,
    " fringilla urna vel, fringilla erat. Vestibulum"_str,
    " malesuada libero quis ipsum scelerisque,"_str,
    " placerat pellentesque tortor "
    "facilisis. Nulla facilisi."_str,
    " Sed pharetra velit in rhoncus vulputate. Lorem ipsum dolor sit amet, ",
    "consectetur adipiscing elit."_str,
    " Aliquam vitae placerat ipsum. Cras nec mi "
    "non odio mattis commodo. "_str,
    "Nulla aliquam odio tortor, at faucibus ligula "
    "malesuada at."_str,
    " Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
    "posuere cubilia curae; Morbi eget finibus libero."_str,
    "Mauris at ante nulla."_str,
    " Fusce felis arcu, vehicula molestie commodo ac,"_str,
    " "
    "aliquam eu sem. Sed congue consequat arcu"_str,
    " at vehicula. Vivamus porta "
    "rutrum mi,"_str,
    " ac sollicitudin nulla suscipit ac."_str,
    " Vestibulum quis magna lorem. "
    "Pellentesque habitant morbi tristique"_str,
    " senectus et netus et malesuada "
    "fames ac turpis egestas."_str,
    " Vestibulum auctor, massa non"_str,
    " hendrerit bibendum, lorem ante laoreet mi, "
    "quis lobortis ipsum odio a velit."_str,
    " Vestibulum auctor, massa non hendrerit bibendum, lorem ante laoreet mi, "
    "quis lobortis ipsum odio a velit."_str,
    " Suspendisse eget aliquam orci."_str};

static void BM_Map_Probe32(benchmark::State & state)
{
  StrMap<i64, u32> map;
  i64 const        num_inserts = state.range(0);
  i64              num_queries = 0;

  for (auto _ : state)
  {
    for (i64 i = 0; i < num_inserts; i++)
    {
      auto & dp = DATASET[i % size(DATASET)];
      map.insert(dp, 0).unwrap();
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
  StrMap<i64, u64> map;
  i64 const        num_inserts = state.range(0);
  i64              num_queries = 0;

  for (auto _ : state)
  {
    for (i64 i = 0; i < num_inserts; i++)
    {
      auto & dp = DATASET[i % size(DATASET)];
      map.insert(dp, 0).unwrap();
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
  std::unordered_map<Span<char const>, i64, StrHasher, StrEq,
                     std_allocator<std::pair<Span<char const> const, i64>>>
            map;
  i64 const num_inserts = state.range(0);
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
struct std::less<Span<char const>>
{
  bool operator()(Span<char const> a, Span<char const> b) const
  {
    std::string_view a_str{a.data(), a.size()};
    std::string_view b_str{b.data(), b.size()};
    return a_str < b_str;
  }
};

template <>
struct std::hash<Span<char const>>
{
  std::hash<std::string_view> hash;

  size_t operator()(Span<char const> str) const
  {
    return hash(std::string_view{str.data(), str.size()});
  }
};

static void BM_StdMapDefaultHash(benchmark::State & state)
{
  std::unordered_map<Span<char const>, i64, std::hash<Span<char const>>, StrEq,
                     std_allocator<std::pair<Span<char const> const, i64>>>
            map;
  i64 const num_inserts = state.range(0);
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
  std::unordered_map<Span<char const>, i64, std::hash<Span<char const>>, StrEq>
            map;
  i64 const num_inserts = state.range(0);
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
  std::map<Span<char const>, i64> map;
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

#define ADD_BENCH(name)             \
  BENCHMARK(BM_##name)              \
      ->Name(#name)                 \
      ->Arg(std::size(DATASET) * 4) \
      ->Iterations(1LL << 16)

ADD_BENCH(Map_Probe32);
ADD_BENCH(Map_Probe64);
ADD_BENCH(StdMap_AshHasher);
ADD_BENCH(StdMapDefaultHash);
ADD_BENCH(StdMapDefaultHashDefaultAlloc);

BENCHMARK_MAIN();
