/*
 * Nico v1.0 - Intérprete Educativo de Scripting en Español
 * @file:         funciones.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Gestión de funciones con retorno, scopes locales, 
 *                parámetros, anidamiento y validación estructural runtime.
 */
#include "nico.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* Prototipos GPIO (necesarios para el puente de funcionalidad) */
void procesar_gpio_configurar(const char *argumento);
void procesar_gpio_estado_pin(const char *argumento);
void procesar_gpio_leer(const char *argumento);

/* VARIABLES GLOBALES DE FUNCIONES */
FuncionInfo funciones_registradas[MAX_NESTING];
int num_funciones_registradas = 0;
double valor_retorno_funcion = 0;
int hay_valor_retorno = 0;
int si_bloque_verdadero_activo = 0;
int si_fin_si_pendiente = -1;
int profundidad_funcion = 0;

/* GESTIÓN DE SCOPES LOCALES */
ScopeLocal scopes_locales[MAX_SCOPES];
int scope_actual = -1;

int crear_scope_local(int funcion_idx) {
    if (!en_funcion) return -1;
    if (scope_actual + 1 >= MAX_SCOPES) return -1;
    scope_actual++;
    scopes_locales[scope_actual].num_variables = 0;
    scopes_locales[scope_actual].funcion_idx = funcion_idx;
    return scope_actual;
}

void eliminar_scope_local(void) {
    if (scope_actual >= 0 && scope_actual < MAX_SCOPES) {
        scopes_locales[scope_actual].num_variables = 0;
        scopes_locales[scope_actual].num_textos = 0;
        scopes_locales[scope_actual].num_listas = 0;
        scopes_locales[scope_actual].num_matrices = 0;
        scope_actual--;
    }
    if (scope_actual < -1) scope_actual = -1;
}

/* GESTIÓN DE LISTAS Y MATRICES LOCALES */
int buscar_lista_local(const char *nombre, int *tipo, int *indice_pool) {
    for (int s = scope_actual; s >= 0; s--) {
        ScopeLocal *scp = &scopes_locales[s];
        for (int i = 0; i < scp->num_listas; i++) {
            if (strcmp(scp->nombres_listas[i], nombre) == 0) {
                if (tipo) *tipo = scp->tipos_listas[i];
                if (indice_pool) *indice_pool = scp->indices_listas[i];
                return 1;
            }
        }
    }
    return 0;
}

int registrar_lista_local(const char *nombre, int tipo, int indice_pool, int capacidad) {
    if (scope_actual < 0) return -1;
    ScopeLocal *scp = &scopes_locales[scope_actual];
    if (scp->num_listas >= MAX_LISTAS_LOCALES) return -1;
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    strncpy(scp->nombres_listas[scp->num_listas], clean, MAX_NOMBRE - 1);
    scp->tipos_listas[scp->num_listas] = tipo;
    scp->indices_listas[scp->num_listas] = indice_pool;
    scp->capacidades_listas[scp->num_listas] = capacidad;
    scp->num_listas++;
    return 0;
}

int buscar_matriz_local(const char *nombre, int *tipo, int *indice_pool) {
    for (int s = scope_actual; s >= 0; s--) {
        ScopeLocal *scp = &scopes_locales[s];
        for (int i = 0; i < scp->num_matrices; i++) {
            if (strcmp(scp->nombres_matrices[i], nombre) == 0) {
                if (tipo) *tipo = scp->tipos_matrices[i];
                if (indice_pool) *indice_pool = scp->indices_matrices[i];
                return 1;
            }
        }
    }
    return 0;
}

int registrar_matriz_local(const char *nombre, int tipo, int indice_pool, int filas, int cols) {
    if (scope_actual < 0) return -1;
    ScopeLocal *scp = &scopes_locales[scope_actual];
    if (scp->num_matrices >= MAX_MATRICES_LOCALES) return -1;
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    strncpy(scp->nombres_matrices[scp->num_matrices], clean, MAX_NOMBRE - 1);
    scp->tipos_matrices[scp->num_matrices] = tipo;
    scp->indices_matrices[scp->num_matrices] = indice_pool;
    scp->filas_matrices[scp->num_matrices] = filas;
    scp->cols_matrices[scp->num_matrices] = cols;
    scp->num_matrices++;
    return 0;
}

int agregar_variable_local(const char *nombre, int tipo, double valor) {
    if (scope_actual < 0) {
        switch(tipo) {
            case 0: return agregar_variable(nombre, (int)valor);
            case 1: return agregar_variable_sin_signo(nombre, (unsigned int)valor);
            case 2: case 3: return agregar_variable_decimal(nombre, valor);
            case 4: return agregar_variable_caracter(nombre, (char)valor);
            case 5: return agregar_variable_caracter_sin_signo(nombre, (unsigned char)valor);
            default: return -1;
        }
    }
    ScopeLocal *scope = &scopes_locales[scope_actual];
    if (scope->num_variables >= MAX_VARS_LOCALES) return -1;
    VariableLocal *var = &scope->variables[scope->num_variables];
    strncpy(var->nombre, nombre, MAX_NOMBRE - 1);
    var->nombre[MAX_NOMBRE - 1] = '\0';
    var->tipo = tipo;
    switch (tipo) {
        case 0: var->valor.valor_entero = (int)valor; break;
        case 1: var->valor.valor_sin_signo = (unsigned int)valor; break;
        case 2: case 3: var->valor.valor_decimal = valor; break;
        case 4: var->valor.valor_caracter = (char)valor; break;
        case 5: var->valor.valor_caracter_sin_signo = (unsigned char)valor; break;
    }
    scope->num_variables++;
    return 0;
}

int buscar_variable_local(const char *nombre, int *tipo, double *valor) {
    if (scope_actual < 0 || scope_actual >= MAX_SCOPES) return 0;
    ScopeLocal *scope = &scopes_locales[scope_actual];
    for (int i = 0; i < scope->num_variables; i++) {
        if (strcmp(scope->variables[i].nombre, nombre) == 0) {
            *tipo = scope->variables[i].tipo;
            switch (scope->variables[i].tipo) {
                case 0: *valor = (double)scope->variables[i].valor.valor_entero; break;
                case 1: *valor = (double)scope->variables[i].valor.valor_sin_signo; break;
                case 2: case 3: *valor = scope->variables[i].valor.valor_decimal; break;
                case 4: *valor = (double)scope->variables[i].valor.valor_caracter; break;
                case 5: *valor = (double)scope->variables[i].valor.valor_caracter_sin_signo; break;
            }
            return 1;
        }
    }
    return 0;
}

/* PARSEAR PARAMETROS DE FUNCION */
int parsear_parametros_funcion(const char *linea, char params[][MAX_NOMBRE], int max_params) {
    int num_params = 0;
    const char *ptr = strchr(linea, '(');
    if (!ptr) return 0;
    ptr++;
    while (*ptr && *ptr != ')' && num_params < max_params) {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '$') {
            ptr++;
            char param[MAX_NOMBRE];
            int i = 0;
            while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) param[i++] = *ptr++;
            param[i] = '\0';
            if (strlen(param) > 0) {
                strncpy(params[num_params++], param, MAX_NOMBRE - 1);
            }
        }
        while (*ptr == ' ' || *ptr == '\t' || *ptr == ',') ptr++;
    }
    return num_params;
}

int obtener_tipo_retorno_funcion(const char *linea) {
    if (strstr(linea, "FUNCION ENTERA SIN SIGNO")) return 1;
    if (strstr(linea, "FUNCION ENTERA")) return 0;
    if (strstr(linea, "FUNCION DECIMAL SIN SIGNO")) return 3;
    if (strstr(linea, "FUNCION DECIMAL")) return 2;
    return -1;
}

/* REGISTRO DE FUNCIONES */
int registrar_todas_las_funciones(void) {
    int error_en_funcion = 0;
    for (int i = 0; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        if (comienza_con(linea, "FUNCION")) {
            int tipo_retorno = obtener_tipo_retorno_funcion(linea);
            if (tipo_retorno == -1) {
                continue;
            }
            const char *ptr = linea;
            if (strstr(linea, "SIN SIGNO")) ptr = strstr(linea, "SIN SIGNO") + 9;
            else if (strstr(linea, "ENTERA")) ptr = strstr(linea, "ENTERA") + 6;
            else if (strstr(linea, "DECIMAL")) ptr = strstr(linea, "DECIMAL") + 7;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            char nombre_func[MAX_NOMBRE];
            int j = 0;
            while (*ptr && *ptr != '(' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1)
                nombre_func[j++] = *ptr++;
            nombre_func[j] = '\0';
            char params[MAX_PARAMETROS][MAX_NOMBRE];
            int num_params = parsear_parametros_funcion(linea, params, MAX_PARAMETROS);
            int fin = encontrar_fin_funcion(i);
            if (fin != -1) {
                int tiene_retornar = 0;
                for (int k = i; k <= fin; k++) {
                    char linea_check[MAX_LINEA];
                    strncpy(linea_check, lineas_programa[k], MAX_LINEA - 1);
                    linea_check[MAX_LINEA - 1] = '\0';
                    limpiar_string(linea_check);
                    remover_comentario(linea_check);
                    if (comienza_con(linea_check, "RETORNAR")) {
                        tiene_retornar = 1;
                        break;
                    }
                }
                if (!tiene_retornar) {
                    fprintf(stderr, "Error línea %d: Función '$%s' debe contener al menos un RETORNAR.\n", i + 1, nombre_func);
                    error_en_funcion = 1;
                    continue;
                }
                registrar_funcion(nombre_func, i, fin, params, num_params, tipo_retorno);
            }
        }
    }
    return error_en_funcion;
}

int registrar_funcion(const char *nombre, int linea_inicio, int linea_fin, 
                      char params[][MAX_NOMBRE], int num_params, int tipo_retorno) {
    if (num_funciones_registradas >= MAX_NESTING) return -1;
    if (num_params > MAX_PARAMETROS) return -1;
    
    strncpy(funciones_registradas[num_funciones_registradas].nombre, nombre, MAX_NOMBRE - 1);
    funciones_registradas[num_funciones_registradas].linea_inicio = linea_inicio;
    funciones_registradas[num_funciones_registradas].linea_fin = linea_fin;
    funciones_registradas[num_funciones_registradas].num_params = num_params;
    funciones_registradas[num_funciones_registradas].tipo_retorno = tipo_retorno;
    
    for (int i = 0; i < num_params; i++) {
        strncpy(funciones_registradas[num_funciones_registradas].params[i], params[i], MAX_NOMBRE - 1);
        funciones_registradas[num_funciones_registradas].tipos_params[i] = tipo_retorno;
    }
    
    num_funciones_registradas++;
    return 0;
}

int buscar_funcion_info(const char *nombre) {
    for (int i = 0; i < num_funciones_registradas; i++) {
        if (strcmp(funciones_registradas[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_inicio_funcion(const char *nombre) {
    int idx = buscar_funcion_info(nombre);
    if (idx >= 0) return funciones_registradas[idx].linea_inicio;
    return -1;
}

/* DECLARAR PARAMETROS DE FUNCION */
int declarar_parametros_funcion(int func_idx, char *args[], int num_args) {
    for (int i = 0; i < num_args && i < funciones_registradas[func_idx].num_params; i++) {
        int exito_arg;
        double valor = evaluar_expresion_completa(args[i], &exito_arg);
        if (!exito_arg) continue;
        
        char *nombre_param = funciones_registradas[func_idx].params[i];
        int tipo_param = funciones_registradas[func_idx].tipos_params[i]; 
        agregar_variable_local(nombre_param, tipo_param, valor);
    }
    return 0;
}

/* EJECUTAR RETORNAR */
int ejecutar_retornar(const char *valor, int linea_actual) {
    (void)linea_actual;
    char valor_limpio[MAX_LINEA];
    strncpy(valor_limpio, valor, MAX_LINEA - 1);
    valor_limpio[MAX_LINEA - 1] = '\0';
    limpiar_string(valor_limpio);
    int exito;
    double resultado = evaluar_expresion_completa(valor_limpio, &exito);
    if (exito) {
        valor_retorno_funcion = resultado;
        hay_valor_retorno = 1;
        return 0;
    } else {
        return -1;
    }
}

int encontrar_fin_si_con_limite(int start_idx, int linea_fin, int limite_superior) {
    int nivel = 1;
    for (int i = start_idx; i <= linea_fin; i++) {
        if (limite_superior != -1 && i > limite_superior) {
            return -1;
        }
        
        char linea_busqueda[MAX_LINEA];
        strncpy(linea_busqueda, lineas_programa[i], MAX_LINEA - 1);
        linea_busqueda[MAX_LINEA - 1] = '\0';
        limpiar_string(linea_busqueda);
        remover_comentario(linea_busqueda);
        
        if (comienza_con(linea_busqueda, "SI") && !comienza_con(linea_busqueda, "SINO") && strstr(linea_busqueda, "ENTONCES")) {
            nivel++;
        }
        else if (strncmp(linea_busqueda, "FIN SI", 6) == 0) {
            nivel--;
            if (nivel == 0) return i; 
        }
    }
    return -1;
}

extern int scope_actual, en_funcion, profundidad_funcion;
extern int hay_valor_retorno;
extern double valor_retorno_funcion;
extern ScopeLocal scopes_locales[];

double llamar_funcion(const char *nombre_func, char *args[], int num_args, int *exito) {
    *exito = 0;
     int func_idx = buscar_funcion_info(nombre_func);
    if (num_args != funciones_registradas[func_idx].num_params) {
        return 0;
    }

    double valores_evaluados[MAX_PARAMETROS];
    for (int k = 0; k < num_args && k < MAX_PARAMETROS; k++) {
        int ex_arg = 0;
        valores_evaluados[k] = evaluar_expresion_completa(args[k], &ex_arg);
    }

    int prev_scope = scope_actual;
    int prev_en_funcion = en_funcion;
    
    en_funcion = 1;
    hay_valor_retorno = 0;
    valor_retorno_funcion = 0;

    if (crear_scope_local(func_idx) < 0) { 
        en_funcion = prev_en_funcion; 
        return 0; 
    }
    
    for (int i = 0; i < num_args && i < funciones_registradas[func_idx].num_params; i++) {
        agregar_variable_local(funciones_registradas[func_idx].params[i], 
                               funciones_registradas[func_idx].tipos_params[i], 
                               valores_evaluados[i]);
    }

    int lim = funciones_registradas[func_idx].linea_fin;
    if (lim <= funciones_registradas[func_idx].linea_inicio) lim = -1;

    CtxBloque ctx_func = {
        .linea_num = funciones_registradas[func_idx].linea_inicio + 1,
        .linea_limite = lim,
        .inicio_encontrado = 1,
        .en_bloque_principal = 0,
        .en_subprograma = 0,
        .en_funcion = 1,
        .fase_declaraciones = 1,
        .declaraciones_permitidas = 1,  
        .error_fatal = 0
    };

     ejecutar_bloque(&ctx_func);

    *exito = 1;
    eliminar_scope_local();
    scope_actual = prev_scope;
    en_funcion = prev_en_funcion;
    
    return hay_valor_retorno ? valor_retorno_funcion : 0.0;
}

/* FUNCIONES ALEATORIAS */
static int semilla_inicializada = 0;

void inicializar_semilla_aleatoria(void) {
    if (!semilla_inicializada) {
        srand((unsigned int)time(NULL));
        semilla_inicializada = 1;
    }
}

double nico_aleatorio_entero(int min, int max) {
    inicializar_semilla_aleatoria();
    if (min > max) { int temp = min; min = max; max = temp; }
    return (double)(rand() % (max - min + 1) + min);
}

double nico_aleatorio_decimal(double min, double max) {
    inicializar_semilla_aleatoria();
    if (min > max) { double temp = min; min = max; max = temp; }
    return min + ((double)rand() / (double)RAND_MAX) * (max - min);
}

double nico_aleatorio_caracter(char min, char max) {
    inicializar_semilla_aleatoria();
    if (min > max) { char temp = min; min = max; max = temp; }
    return (double)(rand() % (max - min + 1) + min);
}

unsigned int nico_aleatorio_sin_signo(unsigned int min, unsigned int max) {
    inicializar_semilla_aleatoria();
    if (min > max) { unsigned int temp = min; min = max; max = temp; }
    return (rand() % (max - min + 1)) + min;
}
