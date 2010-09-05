package = "ao"
version = "unstable-git"
source = {
  url = "git://github.com/TheLinx/lao.git"
}
description = {
  summary = "A library for audio output through Lua.";
  homepage = "http://github.com/TheLinx/lao";
  license = "CC0";
}
dependencies = {
  "lua >= 5.1"
}
external_dependencies = {
  LIBAO = {
    header = "ao/ao.h"
  }
}
build = {
  type = "builtin";
  modules = {
    ao = {
      sources = {"src/lao.c"};
      libraries = {"ao"};
      incdirs = {"$(LIBAO_INCDIR)"};
      libdirs = {"$(LIBAO_LIBDIR)"}
    }
  }
}