#define LUA_LIB

#include <lua.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdint.h>
#include "filewrapper.h"

struct file_factory {
    lua_State *L;
};

static file_handle
fopen_(struct file_factory *f, const char *filename, const char *mode) {
    lua_State *L = f->L;
    lua_pushvalue(L, 1);
    lua_pushstring(L, filename);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        lua_pop(L, 1);  // pop error object
        return NULL;
    }
    const char *realname = lua_tostring(L, -1);
    FILE *f = fopen(realname, mode);
    lua_pop(L, 1);  // pop realname
    return (file_handle)f;
}

static void
fclose_(struct file_factory *f, file_handle handle) {
    fclose((FILE *)handle);
}

static size_t
fread_(struct file_factory *f, file_handle handle, void *buffer, size_t sz) {
    return fread(buffer, 1, sz, (FILE *)handle);
}

static size_t
fwrite_(struct file_factory *f, file_handle handle, const void *buffer, size_t sz) {
    return fwrite(buffer, 1, sz, (FILE *)handle);
}

static size_t
fseek_(struct file_factory *f, file_handle handle, size_t offset) {
    return fseek((FILE *)handle, offset, SEEK_SET); 
}

static int
lfactory(lua_State *L) {
    struct wrapper {
        struct file_interface i;
        struct file_factory f;
    };

    static struct file_api apis = {
        fopen_,
        fclose_,
        fread_,
        fwrite_,
        fseek_,
    };

    struct wrapper *f = (struct wrapper *)lua_userdatauv(L, sizeof(*f), 1);
    f->f.L = lua_newthread(L);
    lua_setiuservalue(L, -2, 1);
    f->i.api = &apis;

    luaL_checktype(L, 1, LUA_TTABLE);
    if (lua_getfield(L, 1, "preopen") != LUA_TFUNCTION) {
        return luaL_error(L, "Need preopen function");
    }

    lua_xmove(L, f->f.L, 1);

    return 1;
}

LUAMOD_API int
luaopen_file(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "factory", lfactory },
        { NULL, NULL },
    };
    luaL_newlib(L, l);

    return 1;    
}

struct box_file {
    file_handle handle;
};

static int
lopen(lua_State *L) {
    struct file_interface *f = (struct file_interface *)lua_touserdata(L, 1);
    const char * filename = luaL_checkstring(L, 2);
    const char * mode = luaL_checkstring(L, 3);
    file_handle handle = file_open(f, filename, mode);
    if (handle == NULL)
        return luaL_error(L, "Can't open %s", filename);
    struct box_file *f = lua_newuserdatauv(L, sizeof(*f), 0);
    f->handle = handle;
    return 1;     
}

LUAMOD_API int
luaopen_file_test(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "open", lopen },
        { NULL, NULL },
    };
    luaL_newlib(L, l);
    return 1;
}
