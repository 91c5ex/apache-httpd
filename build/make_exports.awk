
BEGIN {
    printf("/*\n")
    printf(" * THIS FILE WAS AUTOGENERATED BY make_exports.awk\n")
    printf(" *\n")
    printf(" * This is an ugly hack that needs to be here, so\n")
    printf(" * that libtool will link all of the APR functions\n")
    printf(" * into server regardless of whether the base server\n")
    printf(" * uses them.\n")
    printf(" */\n")
    printf("\n")
    printf("#define CORE_PRIVATE\n")
    printf("\n")
    
    for (i = 1; i < ARGC; i++) {
        file = ARGV[i]
        sub("([^/]*[/])*", "", file)
        printf("#include \"%s\"\n", file)
    }

    printf("\n")
    printf("const void *ap_ugly_hack = NULL;\n")
    printf("\n")
    
    TYPE_NORMAL = 0
    TYPE_HEADER = 1

    stackptr = 0
}

function push(line) {
    stack[stackptr] = line
    stackptr++
}

function do_output() {
    printf("/*\n")
    printf(" * %s\n", FILENAME)
    printf(" */\n")
    
    for (i = 0; i < stackptr; i++) {
        printf("%s\n", stack[i])
    }
    
    stackptr = 0

    printf("\n");
}

function enter_scope(type) {
    scope++
    scope_type[scope] = type
    scope_stack[scope] = stackptr
    delete scope_used[scope]
}

function leave_scope() {
    used = scope_used[scope]
   
    if (!used)
        stackptr = scope_stack[scope]

    scope--
    if (used) {
        scope_used[scope] = 1
        
        if (!scope)
            do_output()
    }
}

function add_symbol(symbol) {
    if (!index(symbol, "#")) {
        push("const void *ap_hack_" symbol " = (const void *)" symbol ";")
        scope_used[scope] = 1
    }
}

/^[ \t]*AP[RU]?_(CORE_)?DECLARE[^(]*[(][^)]*[)]([^ ]* )*[^(]+[(]/ {
    sub("[ \t]*AP[RU]?_(CORE_)?DECLARE[^(]*[(][^)]*[)][ \t]*", "")
    sub("[(].*", "")
    sub("([^ ]* (^([ \t]*[(])))+", "")

    add_symbol($0)
    next
}

/^[ \t]*AP_DECLARE_HOOK[^(]*[(][^)]*[)]/ {
    split($0, args, ",")
    symbol = args[2]
    sub("^[ \t]+", "", symbol)
    sub("[ \t]+$", "", symbol)

    add_symbol("ap_hook_" symbol)
    add_symbol("ap_hook_get_" symbol)
    add_symbol("ap_run_" symbol)
    next
}

/^[ \t]*APR_POOL_DECLARE_ACCESSOR[^(]*[(][^)]*[)]/ {
    sub("[ \t]*APR_POOL_DECLARE_ACCESSOR[^(]*[(]", "", $0)
    sub("[)].*$", "", $0)
    add_symbol("apr_" $0 "_pool_get")
    next
}

/^#[ \t]*if(ndef| !defined[(])([^_]*_)*H/ {
    enter_scope(TYPE_HEADER)
    next
}

/^#[ \t]*if([n]?def)? / {
    enter_scope(TYPE_NORMAL)
    push($0)
    next
}

/^#[ \t]*endif/ {
    if (scope_type[scope] == TYPE_NORMAL)
        push($0)
        
    leave_scope()
    next
}

/^#[ \t]*else/ {
    push($0)
    next
}

/^#[ \t]*elif/ {
    push($0)
    next
}


