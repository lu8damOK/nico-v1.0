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

/* LLAMAR FUNCION */
double llamar_funcion(const char *nombre_func, char *args[], int num_args, int *exito) {
    *exito = 0;
    int func_idx = buscar_funcion_info(nombre_func);
    if (func_idx < 0) {
        fprintf(stderr, "Error: Función '$%s' no declarada.\n", nombre_func);
        return 0;
    }
    if (num_args != funciones_registradas[func_idx].num_params) {
        fprintf(stderr, "Error: Función '$%s' espera %d parámetros.\n", nombre_func, funciones_registradas[func_idx].num_params);
        return 0;
    }

    typedef struct {
        int linea_apertura;      
        int linea_cierre_esperada;
    } SI_Activo;
    
    SI_Activo si_ejecucion[MAX_NESTING];
    int si_ejecucion_ptr = 0;
    int si_structural_ptr = 0;

    profundidad_funcion++;
    en_funcion = 1;
    char destino_anterior[MAX_NOMBRE];
    strncpy(destino_anterior, funcion_variable_destino, MAX_NOMBRE - 1);
    destino_anterior[MAX_NOMBRE - 1] = '\0';
    int retorno_anterior = hay_valor_retorno;
    double valor_anterior = valor_retorno_funcion;
    int stack_ptr_anterior = funcion_destino_stack_ptr;
    
    if (funcion_destino_stack_ptr < MAX_NESTING) {
        strncpy(funcion_destino_stack[funcion_destino_stack_ptr], funcion_variable_destino, MAX_NOMBRE - 1);
        funcion_destino_stack_ptr++;
    }

    double valores_evaluados[MAX_PARAMETROS];
    for (int k = 0; k < num_args && k < MAX_PARAMETROS; k++) {
        double valor_arg = 0;
        int exito_arg = 0;
        const char *arg_raw = args[k];
        while (*arg_raw == ' ' || *arg_raw == '\t') arg_raw++;
        
        if (arg_raw[0] == '$' && !strchr(arg_raw, '[') && !strchr(arg_raw, '+') && !strchr(arg_raw, '-')) {
            char *nombre = (char *)arg_raw + 1;
            int t_loc = -1, i_loc = -1;
            
            if (scope_actual >= 0 && buscar_lista_local(nombre, &t_loc, &i_loc)) {
                valor_arg = 1000000.0 + (t_loc * 10000.0) + i_loc; exito_arg = 1;
            }
            else if (scope_actual >= 0 && buscar_matriz_local(nombre, &t_loc, &i_loc)) {
                valor_arg = 2000000.0 + (t_loc * 10000.0) + i_loc; exito_arg = 1;
            }
            if (!exito_arg) {
                int idx;
                if ((idx = buscar_lista_entera(nombre)) >= 0) { valor_arg = 1000000.0 + (0*10000.0) + idx; exito_arg = 1; }
                else if ((idx = buscar_lista_decimal(nombre)) >= 0) { valor_arg = 1000000.0 + (1*10000.0) + idx; exito_arg = 1; }
                else if ((idx = buscar_lista_entera_sin_signo(nombre)) >= 0) { valor_arg = 1000000.0 + (2*10000.0) + idx; exito_arg = 1; }
                else if ((idx = buscar_lista_decimal_sin_signo(nombre)) >= 0) { valor_arg = 1000000.0 + (3*10000.0) + idx; exito_arg = 1; }
                else if ((idx = buscar_matriz_entera(nombre)) >= 0) { valor_arg = 2000000.0 + (0*10000.0) + idx; exito_arg = 1; }
                else if ((idx = buscar_matriz_decimal(nombre)) >= 0) { valor_arg = 2000000.0 + (1*10000.0) + idx; exito_arg = 1; }
                else if ((idx = buscar_matriz_entera_sin_signo(nombre)) >= 0) { valor_arg = 2000000.0 + (2*10000.0) + idx; exito_arg = 1; }
                else if ((idx = buscar_matriz_decimal_sin_signo(nombre)) >= 0) { valor_arg = 2000000.0 + (3*10000.0) + idx; exito_arg = 1; }
                else if ((idx = buscar_matriz_caracter(nombre)) >= 0) { valor_arg = 2000000.0 + (4*10000.0) + idx; exito_arg = 1; }
                else if ((idx = buscar_matriz_caracter_sin_signo(nombre)) >= 0) { valor_arg = 2000000.0 + (5*10000.0) + idx; exito_arg = 1; }
            }
            
            if (!exito_arg) {
                int idx_txt = buscar_texto_var(nombre);
                if (idx_txt >= 0) {
                    valor_arg = 3000000.0 + (double)idx_txt;
                    exito_arg = 1;
                }
            }
            
            if (!exito_arg && scope_actual >= 0) {
                int tipo_var; double val_var;
                if (buscar_variable_local(nombre, &tipo_var, &val_var) && val_var >= 1000000.0) {
                    valor_arg = val_var; exito_arg = 1;
                }
            }
        }
        
        if (!exito_arg) {
            int scope_backup = scope_actual;
            valor_arg = evaluar_expresion_completa(args[k], &exito_arg);
            scope_actual = scope_backup;
        }
        
        if (!exito_arg) {
            fprintf(stderr, "Error: Expresión inválida en argumento %d de '%s': '%s'.\n", k+1, nombre_func, args[k]);
            profundidad_funcion--; en_funcion = (profundidad_funcion > 0); *exito = 0; return 0;
        }
        valores_evaluados[k] = valor_arg;
    }

    if (crear_scope_local(func_idx) < 0) {
        fprintf(stderr, "Error: Máximo nesting alcanzado (MAX_SCOPES=%d).\n", MAX_SCOPES);
        profundidad_funcion--;
        en_funcion = (profundidad_funcion > 0);
        *exito = 0;
        return 0;
    }

    for (int i = 0; i < num_args && i < funciones_registradas[func_idx].num_params; i++) {
        char *nombre_param = funciones_registradas[func_idx].params[i];
        int tipo_param = funciones_registradas[func_idx].tipos_params[i]; 
        agregar_variable_local(nombre_param, tipo_param, valores_evaluados[i]);
    }

    int linea_inicio = funciones_registradas[func_idx].linea_inicio + 1;
    int linea_fin = funciones_registradas[func_idx].linea_fin;
    hay_valor_retorno = 0;
    valor_retorno_funcion = 0;
    int idx = linea_inicio;

    validar_estructura_bloques(linea_inicio, linea_fin); 
    
    while (idx <= linea_fin && !hay_valor_retorno) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[idx], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) { idx++; continue; }

        /* RETORNAR */
        if (comienza_con(linea, "RETORNAR")) {
            const char *ptr = linea + 8;
            while (*ptr == ' ') ptr++;
            ejecutar_retornar(ptr, idx + 1);
            
            while (si_structural_ptr > 0) {
                si_structural_ptr--;
                if (si_ejecucion_ptr > 0 && si_ejecucion[si_ejecucion_ptr - 1].linea_apertura <= idx) {
                    si_ejecucion_ptr--;
                }
            }
            
            idx++;
            continue;
        }

        /* MIENTRAS...HACER */
        if (strncmp(linea, "MIENTRAS", 8) == 0 && strstr(linea, "HACER")) {
            int exito_cond = 0, resultado = 0;
            char _cond[MAX_LINEA] = "";
            const char *ptr = linea;
            if (strncmp(ptr, "MIENTRAS ", 9) == 0) ptr += 9;
            const char *hacer = strstr(ptr, " HACER");
            if (hacer && hacer > ptr) {
                int len = hacer - ptr;
                if (len > 0 && len < MAX_LINEA - 1) { strncpy(_cond, ptr, len); _cond[len] = '\0'; }
            } else {
                const char *abi = strchr(ptr, '('), *cer = strrchr(ptr, ')');
                if (abi && cer && cer > abi) {
                    int len = cer - abi - 1;
                    if (len > 0 && len < MAX_LINEA - 1) { strncpy(_cond, abi + 1, len); _cond[len] = '\0'; }
                }
            }
            limpiar_string(_cond);
            resultado = evaluar_condicion(_cond, &exito_cond);            
            if (!exito_cond || !resultado) {
                int nivel = 0;
                for (int b = idx + 1; b <= linea_fin; b++) {
                    char lb[MAX_LINEA]; strncpy(lb, lineas_programa[b], MAX_LINEA-1); lb[MAX_LINEA-1]='\0';
                    limpiar_string(lb); remover_comentario(lb);
                    if (strncmp(lb, "FIN MIENTRAS", 12) == 0) { if (nivel == 0) { idx = b + 1; break; } nivel--; }
                    else if (strncmp(lb, "MIENTRAS", 8) == 0 && strstr(lb, "HACER")) nivel++;
                }
            } else { idx++; }
            continue;
        }

        /* FIN MIENTRAS */
        if (strncmp(linea, "FIN MIENTRAS", 12) == 0) {
            int nivel = 0, while_idx = -1;
            
            for (int b = idx - 1; b >= linea_inicio; b--) {
                char lb[MAX_LINEA]; 
                strncpy(lb, lineas_programa[b], MAX_LINEA-1); 
                lb[MAX_LINEA-1]='\0';
                limpiar_string(lb); 
                remover_comentario(lb);
                if (strncmp(lb, "FIN MIENTRAS", 12) == 0) nivel++;
                else if (strncmp(lb, "MIENTRAS", 8) == 0 && strstr(lb, "HACER")) { 
                    if (nivel == 0) { while_idx = b; break; } 
                    nivel--; 
                }
            }
            
	    if (while_idx != -1) {
                int exito_cond = 0, res = 0;
                char _cond[MAX_LINEA] = "";
                const char *src = lineas_programa[while_idx];
                const char *ptr = src;
                if (strncmp(ptr, "MIENTRAS ", 9) == 0) ptr += 9;
                const char *hacer = strstr(ptr, " HACER");
                if (hacer && hacer > ptr) {
                    int len = hacer - ptr;
                    if (len > 0 && len < MAX_LINEA - 1) { strncpy(_cond, ptr, len); _cond[len] = '\0'; }
                } else {
                    const char *abi = strchr(ptr, '('), *cer = strrchr(ptr, ')');
                    if (abi && cer && cer > abi) {
                        int len = cer - abi - 1;
                        if (len > 0 && len < MAX_LINEA - 1) { strncpy(_cond, abi + 1, len); _cond[len] = '\0'; }
                    }
                }
                limpiar_string(_cond);
                res = evaluar_condicion(_cond, &exito_cond);
                idx = (exito_cond && res) ? (while_idx + 1) : (idx + 1);
            } else { idx++; }
            continue;
        }

        /* CALCULAR EN / ASIGNAR EN / RESULTADO EN */
        if (comienza_con(linea, "CALCULAR EN") || comienza_con(linea, "ASIGNAR EN") || comienza_con(linea, "RESULTADO EN")) {
            const char *expr_ptr = linea;
            if (comienza_con(linea, "CALCULAR EN")) expr_ptr += 11;
            else if (comienza_con(linea, "ASIGNAR EN")) expr_ptr += 10;
            else expr_ptr += 12;
            while (*expr_ptr == ' ') expr_ptr++;
    
            const char *igual = strchr(expr_ptr, '=');
            if (igual) {
                char var_nombre[MAX_NOMBRE] = "";
                const char *p = expr_ptr;
                while (p < igual && *p != '$') p++;
                if (*p == '$') p++;
                int i = 0; while (p < igual && es_alnum(*p) && i < MAX_NOMBRE-1) var_nombre[i++] = *p++;
                var_nombre[i] = '\0';
        
                char lado_der[MAX_LINEA]; strncpy(lado_der, igual + 1, MAX_LINEA-1); limpiar_string(lado_der);
                int exito_expr; double valor = evaluar_expresion_completa(lado_der, &exito_expr);
        
                if (exito_expr && scope_actual >= 0) {
                    int actualizado = 0;
            
                    /* DETECTAR MATRIZ 2D: $nombre[fila][columna] */
                    const char *corchete1 = strchr(expr_ptr, '[');
                    if (corchete1) {
                        const char *cierre1 = strchr(corchete1 + 1, ']');
                        const char *corchete2 = cierre1 ? strchr(cierre1 + 1, '[') : NULL;
                
                        if (corchete2) {
                            char fila_str[MAX_LINEA] = "", col_str[MAX_LINEA] = "";
                    
                            const char *p_cor1 = corchete1 + 1;
                            i = 0; int nivel = 1;
                            while (*p_cor1 && nivel > 0 && i < MAX_LINEA - 1) {
                                if (*p_cor1 == '[') nivel++; else if (*p_cor1 == ']') nivel--;
                                if (nivel > 0) fila_str[i++] = *p_cor1;
                                p_cor1++;
                            }
                            fila_str[i] = '\0'; limpiar_string(fila_str);
                    
                            const char *p_cor2 = corchete2 + 1;
                            i = 0; nivel = 1;
                            while (*p_cor2 && nivel > 0 && i < MAX_LINEA - 1) {
                                if (*p_cor2 == '[') nivel++; else if (*p_cor2 == ']') nivel--;
                                if (nivel > 0) col_str[i++] = *p_cor2;
                                p_cor2++;
                            }
                            col_str[i] = '\0'; limpiar_string(col_str);
                    
                            int ef, ec;
                            double vf = evaluar_expresion_completa(fila_str, &ef);
                            double vc = evaluar_expresion_completa(col_str, &ec);
                            int fila = ef ? (int)vf : atoi(fila_str);
                            int col = ec ? (int)vc : atoi(col_str);
                    
                            int es_ref_mat = 0, tipo_ref_mat = 0, idx_ref_mat = 0;
                            double val_param_mat = 0;
                    
                            for (int sc = scope_actual; sc >= 0; sc--) {
                                ScopeLocal *scp = &scopes_locales[sc];
                                for (int j = 0; j < scp->num_variables; j++) {
                                    if (strcmp(scp->variables[j].nombre, var_nombre) == 0) {
                                        switch(scp->variables[j].tipo) {
                                            case 0: val_param_mat = (double)scp->variables[j].valor.valor_entero; break;
                                            case 1: val_param_mat = (double)scp->variables[j].valor.valor_sin_signo; break;
                                            case 2: case 3: val_param_mat = scp->variables[j].valor.valor_decimal; break;
                                            case 4: val_param_mat = (double)scp->variables[j].valor.valor_caracter; break;
                                            case 5: val_param_mat = (double)scp->variables[j].valor.valor_caracter_sin_signo; break;
                                        }
                                        if (val_param_mat >= 2000000.0 && val_param_mat < 2060000.0) {
                                            int ref = (int)(val_param_mat - 2000000.0);
                                            tipo_ref_mat = ref / 10000;
                                            idx_ref_mat = ref % 10000;
                                            es_ref_mat = 1;
                                        }
                                        break;
                                    }
                                }
                                if (es_ref_mat) break;
                            }
                    
                            if (es_ref_mat) {
                                int filas_max = 0, cols_max = 0;
                                switch(tipo_ref_mat) {
                                    case 0: filas_max = matrices_enteras[idx_ref_mat].filas; cols_max = matrices_enteras[idx_ref_mat].columnas; break;
                                    case 1: case 3: filas_max = matrices_decimales[idx_ref_mat].filas; cols_max = matrices_decimales[idx_ref_mat].columnas; break;
                                    case 2: filas_max = matrices_enteras_sin_signo[idx_ref_mat].filas; cols_max = matrices_enteras_sin_signo[idx_ref_mat].columnas; break;
                                    case 4: filas_max = matrices_caracter[idx_ref_mat].filas; cols_max = matrices_caracter[idx_ref_mat].columnas; break;
                                    case 5: filas_max = matrices_caracter_sin_signo[idx_ref_mat].filas; cols_max = matrices_caracter_sin_signo[idx_ref_mat].columnas; break;
                                }
                                if (fila >= 0 && fila < filas_max && col >= 0 && col < cols_max) {
                                    switch(tipo_ref_mat) {
                                        case 0: matrices_enteras[idx_ref_mat].valores[fila][col] = (int)valor; break;
                                        case 1: case 3: matrices_decimales[idx_ref_mat].valores[fila][col] = valor; break;
                                        case 2: matrices_enteras_sin_signo[idx_ref_mat].valores[fila][col] = (unsigned int)valor; break;
                                        case 4: matrices_caracter[idx_ref_mat].valores[fila][col] = (char)valor; break;
                                        case 5: matrices_caracter_sin_signo[idx_ref_mat].valores[fila][col] = (unsigned char)valor; break;
                                    }
                                    actualizado = 1;
                                }
                            }
                        
                            if (!actualizado) {
                                int tipo_loc = -1, idx_loc = -1;
                                if (buscar_matriz_local(var_nombre, &tipo_loc, &idx_loc)) {
                                    int filas_max = 0, cols_max = 0;
                                    switch(tipo_loc) {
                                        case 0: filas_max = matrices_enteras[idx_loc].filas; cols_max = matrices_enteras[idx_loc].columnas; break;
                                        case 1: case 3: filas_max = matrices_decimales[idx_loc].filas; cols_max = matrices_decimales[idx_loc].columnas; break;
                                        case 2: filas_max = matrices_enteras_sin_signo[idx_loc].filas; cols_max = matrices_enteras_sin_signo[idx_loc].columnas; break;
                                        case 4: filas_max = matrices_caracter[idx_loc].filas; cols_max = matrices_caracter[idx_loc].columnas; break;
                                        case 5: filas_max = matrices_caracter_sin_signo[idx_loc].filas; cols_max = matrices_caracter_sin_signo[idx_loc].columnas; break;
                                    }
                                    if (fila >= 0 && fila < filas_max && col >= 0 && col < cols_max) {
                                        switch(tipo_loc) {
                                            case 0: matrices_enteras[idx_loc].valores[fila][col] = (int)valor; break;
                                            case 1: case 3: matrices_decimales[idx_loc].valores[fila][col] = valor; break;
                                            case 2: matrices_enteras_sin_signo[idx_loc].valores[fila][col] = (unsigned int)valor; break;
                                            case 4: matrices_caracter[idx_loc].valores[fila][col] = (char)valor; break;
                                            case 5: matrices_caracter_sin_signo[idx_loc].valores[fila][col] = (unsigned char)valor; break;
                                        }
                                        actualizado = 1;
                                    }
                                }
                            }
                        
                            if (!actualizado) {
                                if (buscar_matriz_entera(var_nombre) >= 0) { set_matriz_entera_valor(var_nombre, fila, col, (int)valor); actualizado = 1; }
                                else if (buscar_matriz_decimal(var_nombre) >= 0) { set_matriz_decimal_valor(var_nombre, fila, col, valor); actualizado = 1; }
                                else if (buscar_matriz_entera_sin_signo(var_nombre) >= 0) { set_matriz_entera_sin_signo_valor(var_nombre, fila, col, (unsigned int)valor); actualizado = 1; }
                                else if (buscar_matriz_decimal_sin_signo(var_nombre) >= 0) { set_matriz_decimal_sin_signo_valor(var_nombre, fila, col, valor); actualizado = 1; }
                                else if (buscar_matriz_caracter(var_nombre) >= 0) { set_matriz_caracter_valor(var_nombre, fila, col, (char)valor); actualizado = 1; }
                                else if (buscar_matriz_caracter_sin_signo(var_nombre) >= 0) { set_matriz_caracter_sin_signo_valor(var_nombre, fila, col, (unsigned char)valor); actualizado = 1; }
                            }
                        }
                    }
            
                    /* Soporte para LISTAS 1D */
                    if (!actualizado) {
                        const char *corchete = strchr(expr_ptr, '[');
                        if (corchete) {
                            char indice_str[MAX_LINEA] = "";
                            int k = 0; const char *pp = corchete + 1; int nivel = 1;
                            while (*pp && nivel > 0 && k < MAX_LINEA - 1) {
                                if (*pp == '[') nivel++; else if (*pp == ']') nivel--;
                                if (nivel > 0) indice_str[k++] = *pp;
                                pp++;
                            }
                            indice_str[k] = '\0'; limpiar_string(indice_str);
                            int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                            int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                            int es_ref = 0, tipo_ref = 0, idx_ref = 0;
                            double val_param = 0;
                            if (scope_actual >= 0 && scope_actual < MAX_SCOPES) {
                                for (int sc = scope_actual; sc >= 0; sc--) {
                                    ScopeLocal *scp = &scopes_locales[sc];
                                    for (int j = 0; j < scp->num_variables; j++) {
                                        if (strcmp(scp->variables[j].nombre, var_nombre) == 0) {
                                            switch(scp->variables[j].tipo) {
                                                case 0: val_param = (double)scp->variables[j].valor.valor_entero; break;
                                                case 1: val_param = (double)scp->variables[j].valor.valor_sin_signo; break;
                                                case 2: case 3: val_param = scp->variables[j].valor.valor_decimal; break;
                                                case 4: val_param = (double)scp->variables[j].valor.valor_caracter; break;
                                                case 5: val_param = (double)scp->variables[j].valor.valor_caracter_sin_signo; break;
                                            }
                                            if (val_param >= 1000000.0 && val_param < 1060000.0) {
                                                int ref = (int)(val_param - 1000000.0);
                                                tipo_ref = ref / 10000; idx_ref = ref % 10000; es_ref = 1;
                                                (void)tipo_ref; 
                                                (void)idx_ref;
                                            }
                                            break;
                                        }
                                    }
                                    if (es_ref) break;
                                }
                            }
                    
                            if (!actualizado) {
                                int tipo_loc = -1, idx_loc = -1;
                                if (buscar_lista_local(var_nombre, &tipo_loc, &idx_loc)) {
                                    if (indice >= 0 && indice < MAX_LISTA) {
                                        switch(tipo_loc) {
                                            case 0: listas_enteras[idx_loc].valores[indice] = (int)valor; break;
                                            case 1: case 3: listas_decimales[idx_loc].valores[indice] = valor; break;
                                            case 2: listas_enteras_sin_signo[idx_loc].valores[indice] = (unsigned int)valor; break;
                                            case 4: listas_caracter[idx_loc].valores[indice] = (char)valor; break;
                                            case 5: listas_caracter_sin_signo[idx_loc].valores[indice] = (unsigned char)valor; break;
                                        }
                                        actualizado = 1;
                                    }
                                }
                            }
                        }
                    }
            
                    if (!actualizado) {
                        for (int sc = scope_actual; sc >= 0; sc--) {
                            for (int j = 0; j < scopes_locales[sc].num_variables; j++) {
                                if (strcmp(scopes_locales[sc].variables[j].nombre, var_nombre) == 0) {
                                    switch (scopes_locales[sc].variables[j].tipo) {
                                        case 0: scopes_locales[sc].variables[j].valor.valor_entero = (int)valor; break;
                                        case 1: scopes_locales[sc].variables[j].valor.valor_sin_signo = (unsigned int)valor; break;
                                        case 2: case 3: scopes_locales[sc].variables[j].valor.valor_decimal = valor; break;
                                        case 4: scopes_locales[sc].variables[j].valor.valor_caracter = (char)valor; break;
                                        case 5: scopes_locales[sc].variables[j].valor.valor_caracter_sin_signo = (unsigned char)valor; break;
                                    }
                                    actualizado = 1; break;
                                }
                            }
                            if (actualizado) break;
                        }
                    }
            
                    if (!actualizado) {
                        int idx_v = buscar_variable(var_nombre);
                        if (idx_v >= 0) { variables[idx_v].valor = (int)valor; actualizado = 1; }
                        if (!actualizado) { idx_v = buscar_variable_sin_signo(var_nombre); if (idx_v >= 0) { variables_sin_signo[idx_v].valor = (unsigned int)valor; actualizado = 1; } }
                        if (!actualizado) { idx_v = buscar_variable_decimal(var_nombre); if (idx_v >= 0) { variables_decimal[idx_v].valor = valor; actualizado = 1; } }
                        if (!actualizado) { idx_v = buscar_variable_decimal_sin_signo(var_nombre); if (idx_v >= 0) { variables_decimal_sin_signo[idx_v].valor = valor; actualizado = 1; } }
                        if (!actualizado) { idx_v = buscar_variable_caracter(var_nombre); if (idx_v >= 0) { variables_caracter[idx_v].valor = (char)valor; actualizado = 1; } }
                        if (!actualizado) { idx_v = buscar_variable_caracter_sin_signo(var_nombre); if (idx_v >= 0) { variables_caracter_sin_signo[idx_v].valor = (unsigned char)valor; actualizado = 1; } }
                    }
            
                    if (!actualizado) {
                        fprintf(stderr, "Error: No se pudo asignar a '$%s' (¿variable/matriz no declarada?).\n", var_nombre);
                    }
                }
            }
            idx++;
            continue;
        }

        /* DECLARACIONES */
        if (strncmp(linea, "VARIABLE ", 9) == 0 || strncmp(linea, "DECLARAR ", 9) == 0) {
            if (strstr(linea, "TEXTO")) {
                char var_nombre[MAX_NOMBRE] = "";
                const char *p = strchr(linea, '$');
                if (p) { p++; int k=0; while(*p && es_alnum(*p) && k<MAX_NOMBRE-1) var_nombre[k++]=*p++; var_nombre[k]='\0'; }
                char valor_texto[MAX_TEXTO_LEN] = "";
                const char *igual = strchr(linea, '=');
                if (igual) { igual++; while(*igual==' ') igual++; if (*igual == '"') { igual++; int k=0; while(*igual && *igual!='"' && k<MAX_TEXTO_LEN-1) valor_texto[k++]=*igual++; valor_texto[k]='\0'; } }
                agregar_texto_local(var_nombre, valor_texto);
                idx++; continue;
            }
            int tipo = 0;
            if (strstr(linea, "DECIMAL")) tipo = 2;
            else if (strstr(linea, "CARACTER")) tipo = 4;
            else if (strstr(linea, "SIN SIGNO")) tipo = (strstr(linea, "ENTERA")) ? 1 : 3;
            char var_nombre[MAX_NOMBRE] = "";
            const char *p = strchr(linea, '$');
            if (!p) { idx++; continue; }
            p++; int k = 0;
            while (*p && es_alnum(*p) && k < MAX_NOMBRE-1) var_nombre[k++] = *p++;
            var_nombre[k] = '\0';
            double valor_inicial = 0;
            const char *igual = strchr(p, '=');
            if (igual) { 
                char val_str[MAX_LINEA] = ""; 
                strncpy(val_str, igual + 1, MAX_LINEA-1); 
                limpiar_string(val_str); 
                int exito_val = 0;
                valor_inicial = evaluar_expresion_completa(val_str, &exito_val);
                if (!exito_val) valor_inicial = atof(val_str);
            }
            if (scope_actual >= 0 && scope_actual < MAX_SCOPES) {
                ScopeLocal *scp = &scopes_locales[scope_actual];
                int var_existente = -1;
                for (int v = 0; v < scp->num_variables; v++) { if (strcmp(scp->variables[v].nombre, var_nombre) == 0) { var_existente = v; break; } }
                if (scp->num_variables < MAX_VARS_LOCALES) {
                    strncpy(scp->variables[scp->num_variables].nombre, var_nombre, MAX_NOMBRE-1);
                    scp->variables[scp->num_variables].tipo = tipo;
                    switch(tipo) {
                        case 0: scp->variables[scp->num_variables].valor.valor_entero = (int)valor_inicial; break;
                        case 1: scp->variables[scp->num_variables].valor.valor_sin_signo = (unsigned int)valor_inicial; break;
                        case 2: case 3: scp->variables[scp->num_variables].valor.valor_decimal = valor_inicial; break;
                        case 4: scp->variables[scp->num_variables].valor.valor_caracter = (char)valor_inicial; break;
                        case 5: scp->variables[scp->num_variables].valor.valor_caracter_sin_signo = (unsigned char)valor_inicial; break;
                    }
                    scp->num_variables++;
                }
                if (var_existente >= 0) {
                    switch(tipo) {
                        case 0: scp->variables[var_existente].valor.valor_entero = (int)valor_inicial; break;
                        case 1: scp->variables[var_existente].valor.valor_sin_signo = (unsigned int)valor_inicial; break;
                        case 2: case 3: scp->variables[var_existente].valor.valor_decimal = valor_inicial; break;
                        case 4: scp->variables[var_existente].valor.valor_caracter = (char)valor_inicial; break;
                        case 5: scp->variables[var_existente].valor.valor_caracter_sin_signo = (unsigned char)valor_inicial; break;
                    }
                }
            }
            idx++; continue;
        }
       
        /* DECLARACIONES DE LISTAS Y MATRICES LOCALES */
        if (strncmp(linea, "LISTA ", 6) == 0 || strncmp(linea, "MATRIZ ", 7) == 0 || strncmp(linea, "DECLARAR LISTA ", 15) == 0 || strncmp(linea, "DECLARAR MATRIZ ", 16) == 0) {
            if (strstr(linea, "LISTA ENTERA SIN SIGNO") || strstr(linea, "DECLARAR LISTA ENTERA SIN SIGNO")) procesar_declaracion_lista_entera_sin_signo(linea, idx + 1);
            else if (strstr(linea, "LISTA DECIMAL SIN SIGNO") || strstr(linea, "DECLARAR LISTA DECIMAL SIN SIGNO")) procesar_declaracion_lista_decimal_sin_signo(linea, idx + 1);
            else if (strstr(linea, "LISTA CARACTER SIN SIGNO") || strstr(linea, "DECLARAR LISTA CARACTER SIN SIGNO")) procesar_declaracion_lista_caracter_sin_signo(linea, idx + 1);
            else if (strstr(linea, "LISTA CARACTER") || strstr(linea, "DECLARAR LISTA CARACTER")) procesar_declaracion_lista_caracter(linea, idx + 1);
            else if (strstr(linea, "LISTA ENTERA") || strstr(linea, "DECLARAR LISTA ENTERA")) procesar_declaracion_lista_entera(linea, idx + 1);
            else if (strstr(linea, "LISTA DECIMAL") || strstr(linea, "DECLARAR LISTA DECIMAL")) procesar_declaracion_lista_decimal(linea, idx + 1);
            else if (strstr(linea, "MATRIZ ENTERA SIN SIGNO") || strstr(linea, "DECLARAR MATRIZ ENTERA SIN SIGNO")) procesar_declaracion_matriz_entera_sin_signo(linea, idx + 1);
            else if (strstr(linea, "MATRIZ DECIMAL SIN SIGNO") || strstr(linea, "DECLARAR MATRIZ DECIMAL SIN SIGNO")) procesar_declaracion_matriz_decimal_sin_signo(linea, idx + 1);
            else if (strstr(linea, "MATRIZ CARACTER SIN SIGNO") || strstr(linea, "DECLARAR MATRIZ CARACTER SIN SIGNO")) procesar_declaracion_matriz_caracter_sin_signo(linea, idx + 1);
            else if (strstr(linea, "MATRIZ CARACTER") || strstr(linea, "DECLARAR MATRIZ CARACTER")) procesar_declaracion_matriz_caracter(linea, idx + 1);
            else if (strstr(linea, "MATRIZ ENTERA") || strstr(linea, "DECLARAR MATRIZ ENTERA")) procesar_declaracion_matriz_entera(linea, idx + 1);
            else if (strstr(linea, "MATRIZ DECIMAL") || strstr(linea, "DECLARAR MATRIZ DECIMAL")) procesar_declaracion_matriz_decimal(linea, idx + 1);
            idx++; continue;
        }

        /* ESCRIBIR / MOSTRAR */
        if (comienza_con(linea, "ESCRIBIR") || comienza_con(linea, "MOSTRAR")) {
            const char *ptr = comienza_con(linea, "ESCRIBIR") ? linea + 8 : linea + 7;
            while (*ptr == ' ') ptr++;
            procesar_escribir(ptr);
            idx++;
            continue;
        }

        /* ESPERAR */
        if (comienza_con(linea, "ESPERAR")) {
            const char *ptr = linea + 7;
            while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
            const char *coma = strchr(ptr, ',');
            if (!coma) {
                fprintf(stderr, "Error línea %d: Formato inválido. Uso correcto: ESPERAR(valor, UNIDAD)\n", idx + 1);
                exit(1);
            }
        
            const char *unidad = coma + 1;
            while (*unidad == ' ' || *unidad == '\t') unidad++;
            if (*unidad == '\0' || *unidad == ')') {
                fprintf(stderr, "Error línea %d: Falta unidad de tiempo después de la coma.\n", idx + 1);
                exit(1);
            }

            char *argumento = linea + 7;
            while (*argumento == ' ' || *argumento == '\t') argumento++;
            procesar_esperar(argumento);
            idx++;
            continue;
        }
        
        /* SI / SINO / FIN SI */        
        if (comienza_con(linea, "SI") && !comienza_con(linea, "SINO")) {
            if (!strstr(linea, "ENTONCES")) {
                fprintf(stderr, "Error línea %d: Falta 'ENTONCES' en SI.\n", idx + 1);
                exit(1);
            }
            
            int exito_cond = 0, resultado = 0;
            char _cond[MAX_LINEA] = "";
            const char *ptr = linea;
            if (strncmp(ptr, "SI ", 3) == 0) ptr += 3;
            
            const char *ent = strstr(ptr, " ENTONCES");
            if (!ent) ent = strstr(ptr, " ENT");
            if (ent && ent > ptr) {
                int len = ent - ptr;
                if (len > 0 && len < MAX_LINEA - 1) { strncpy(_cond, ptr, len); _cond[len] = '\0'; }
            } else {
                const char *abi = strchr(ptr, '('), *cer = strrchr(ptr, ')');
                if (abi && cer && cer > abi) {
                    int len = cer - abi - 1;
                    if (len > 0 && len < MAX_LINEA - 1) { strncpy(_cond, abi + 1, len); _cond[len] = '\0'; }
                }
            }
            limpiar_string(_cond);
            resultado = evaluar_condicion(_cond, &exito_cond);            

            if (exito_cond && resultado) {
                int limite_padre = (si_ejecucion_ptr > 0) ? si_ejecucion[si_ejecucion_ptr - 1].linea_cierre_esperada : -1;
                int idx_fin_si = encontrar_fin_si_con_limite(idx + 1, linea_fin, limite_padre);
                if (idx_fin_si == -1) { fprintf(stderr, "Error línea %d: Falta 'FIN SI'.\n", idx + 1); exit(1); }
                for (int i = 0; i < si_ejecucion_ptr; i++) {
                    if (si_ejecucion[i].linea_cierre_esperada == idx_fin_si) {
                        fprintf(stderr, "Error línea %d: 'FIN SI' mal ubicado.\n", idx + 1); exit(1);
                    }
                }
                if (si_ejecucion_ptr >= MAX_NESTING) { fprintf(stderr, "Error línea %d: Anidamiento excedido.\n", idx + 1); exit(1); }
                si_ejecucion[si_ejecucion_ptr].linea_apertura = idx;
                si_ejecucion[si_ejecucion_ptr].linea_cierre_esperada = idx_fin_si;
                si_ejecucion_ptr++; si_structural_ptr++;
                idx++;
            } else {
                si_structural_ptr++;
                int nivel_salto = 0, encontrado = 0;
                for (int b = idx + 1; b <= linea_fin; b++) {
                    char lb[MAX_LINEA]; strncpy(lb, lineas_programa[b], MAX_LINEA - 1); lb[MAX_LINEA - 1] = '\0';
                    limpiar_string(lb); remover_comentario(lb);
                    if (comienza_con(lb, "SI") && !comienza_con(lb, "SINO") && strstr(lb, "ENTONCES")) nivel_salto++;
                    else if (strncmp(lb, "FIN SI", 6) == 0) { if (nivel_salto == 0) { idx = b + 1; encontrado = 1; break; } nivel_salto--; }
                    else if (comienza_con(lb, "SINO") && !comienza_con(lb, "SINO SI")) { if (nivel_salto == 0) { idx = b + 1; encontrado = 1; break; } }
                }
                if (!encontrado) { fprintf(stderr, "Error línea %d: Falta SINO o FIN SI.\n", idx + 1); exit(1); }
            }
            continue;
        }

        if (strncmp(linea, "FIN SI", 6) == 0) {
            if (si_structural_ptr <= 0) {
                fprintf(stderr, "Error línea %d: 'FIN SI' sin 'SI' correspondiente.\n", idx + 1);
                exit(1);
            }
            si_structural_ptr--;
            
            if (si_ejecucion_ptr > 0 && si_ejecucion[si_ejecucion_ptr - 1].linea_cierre_esperada == idx) {
                si_ejecucion_ptr--;
            }
            
            idx++;
            continue;
        }

        if (comienza_con(linea, "SINO") && !comienza_con(linea, "SINO SI")) {
            idx++;
            continue;
        }

        /* Otros handlers */
        if (strncmp(linea, "MIENTRAS", 8) == 0) {
            if (strstr(linea, "HACER")) { idx++; continue; }
            char _cond[MAX_LINEA] = "";
            const char *_a = strchr(linea, '(');
            const char *_c = strrchr(linea, ')');
            if (_a && _c && _c > _a) {
                int _l = _c - _a - 1;
                if (_l > 0 && _l < MAX_LINEA - 1) {
                    strncpy(_cond, _a + 1, _l);
                    _cond[_l] = '\0';
                    limpiar_string(_cond);
                }
            }
            int exito_cond, resultado = evaluar_condicion(_cond, &exito_cond);

            if (exito_cond && resultado) {
                int nivel = 0;
                for (int b = idx - 1; b >= linea_inicio; b--) {
                    char lb[MAX_LINEA]; strncpy(lb, lineas_programa[b], MAX_LINEA-1); lb[MAX_LINEA-1]='\0';
                    limpiar_string(lb); remover_comentario(lb);
                    if (strncmp(lb, "REALIZAR", 8) == 0) { if (nivel == 0) { idx = b + 1; break; } nivel--; }
                    else if (strncmp(lb, "MIENTRAS", 8) == 0 && !strstr(lb, "HACER")) nivel++;
                }
            } else { idx++; }
            continue;
        }

        if (strncmp(linea, "PARA", 4) == 0 && strstr(linea, "HACER")) {
            char var_nombre[MAX_NOMBRE] = ""; double inicio = 0, fin_val = 0, paso = 1; int scope_para = scope_actual;
            const char *apert = strchr(linea, '('); const char *cierre = strrchr(linea, ')');
            if (apert && cierre) {
                char cont[MAX_LINEA]; int len = cierre - apert - 1;
                if (len > 0 && len < MAX_LINEA-1) {
                    strncpy(cont, apert+1, len); cont[len]='\0'; limpiar_string(cont);
                    const char *p = cont; while (*p && *p != '$') p++; if (*p == '$') p++;
                    int k = 0; while (*p && es_alnum(*p) && k < MAX_NOMBRE-1) var_nombre[k++] = *p++; var_nombre[k] = '\0';
                    const char *igual = strchr(cont, '='); const char *hasta = strstr(cont, "HASTA");
                    if (igual && hasta) { char ini_s[MAX_LINEA]; int il = hasta - igual - 1; if (il > 0 && il < MAX_LINEA) { strncpy(ini_s, igual+1, il); ini_s[il]='\0'; limpiar_string(ini_s); int exito_ini; inicio = evaluar_expresion_completa(ini_s, &exito_ini); if (!exito_ini) inicio = atof(ini_s); } }
                    if (hasta) { char fin_s[MAX_LINEA]; const char *paso_ptr = strstr(hasta+5, "PASO"); const char *fin_ptr = paso_ptr ? paso_ptr : (cont + len); int fl = fin_ptr - hasta - 5; if (fl > 0 && fl < MAX_LINEA) { strncpy(fin_s, hasta+5, fl); fin_s[fl]='\0'; limpiar_string(fin_s); int exito_fin; fin_val = evaluar_expresion_completa(fin_s, &exito_fin); if (!exito_fin) fin_val = atof(fin_s); } }
                    const char *paso_ptr = strstr(cont, "PASO");
                    if (paso_ptr) { char ps_s[MAX_LINEA]; int pl = (cont + len) - (paso_ptr + 4); if (pl > 0 && pl < MAX_LINEA) { strncpy(ps_s, paso_ptr+4, pl); ps_s[pl]='\0'; limpiar_string(ps_s); int exito_paso; paso = evaluar_expresion_completa(ps_s, &exito_paso); if (!exito_paso) paso = atof(ps_s); if (paso == 0) paso = (fin_val >= inicio) ? 1 : -1; } }
                }
            }
            if (scope_para >= 0 && strlen(var_nombre) > 0) {
                ScopeLocal *scp = &scopes_locales[scope_para]; int found = 0;
                for (int j = 0; j < scp->num_variables; j++) {
                    if (strcmp(scp->variables[j].nombre, var_nombre) == 0) { found = 1; switch(scp->variables[j].tipo) { case 0: scp->variables[j].valor.valor_entero = (int)inicio; break; case 1: scp->variables[j].valor.valor_sin_signo = (unsigned int)inicio; break; case 2: case 3: scp->variables[j].valor.valor_decimal = inicio; break; case 4: scp->variables[j].valor.valor_caracter = (char)inicio; break; case 5: scp->variables[j].valor.valor_caracter_sin_signo = (unsigned char)inicio; break; } break; }
                }
                if (!found && scp->num_variables < MAX_VARS_LOCALES) { strncpy(scp->variables[scp->num_variables].nombre, var_nombre, MAX_NOMBRE-1); scp->variables[scp->num_variables].tipo = 2; scp->variables[scp->num_variables].valor.valor_decimal = inicio; scp->num_variables++; }
            }
            int entrar = (paso > 0) ? (inicio <= fin_val) : (inicio >= fin_val);
            if (!entrar) { int nivel = 0; for (int b = idx + 1; b <= linea_fin; b++) { char lb[MAX_LINEA]; strncpy(lb, lineas_programa[b], MAX_LINEA-1); lb[MAX_LINEA-1]='\0'; limpiar_string(lb); remover_comentario(lb); if (strncmp(lb, "FIN PARA", 8) == 0) { if (nivel == 0) { idx = b + 1; break; } nivel--; } else if (strncmp(lb, "PARA", 4) == 0 && strstr(lb, "HACER")) nivel++; } }
            idx++; continue;
        }

        if (strncmp(linea, "FIN PARA", 8) == 0) {
            int nivel = 0, para_idx = -1;
            for (int b = idx - 1; b >= linea_inicio; b--) { char lb[MAX_LINEA]; strncpy(lb, lineas_programa[b], MAX_LINEA-1); lb[MAX_LINEA-1]='\0'; limpiar_string(lb); remover_comentario(lb); if (strncmp(lb, "FIN PARA", 8) == 0) nivel++; else if (strncmp(lb, "PARA", 4) == 0 && strstr(lb, "HACER")) { if (nivel == 0) { para_idx = b; break; } nivel--; } }
            if (para_idx != -1) {
                char var_n[MAX_NOMBRE] = ""; double fin_v = 0, pas = 1;
                const char *a = strchr(lineas_programa[para_idx], '('); const char *c = strrchr(lineas_programa[para_idx], ')');
                if (a && c) { char ct[MAX_LINEA]; int l = c - a - 1; if (l > 0 && l < MAX_LINEA) { strncpy(ct, a+1, l); ct[l]='\0'; limpiar_string(ct); const char *p = ct; while (*p && *p != '$') p++; if (*p == '$') p++; int k2 = 0; while (*p && es_alnum(*p) && k2 < MAX_NOMBRE-1) var_n[k2++] = *p++; var_n[k2] = '\0'; const char *hp = strstr(ct, "HASTA"); if (hp) { const char *pp = strstr(hp+5, "PASO"); int fl = pp ? (pp-hp-5) : (ct+l - hp - 5); if (fl > 0) { char fs[MAX_LINEA]; strncpy(fs, hp+5, fl); fs[fl]='\0'; limpiar_string(fs); int exito_fin; double val_fin = evaluar_expresion_completa(fs, &exito_fin); fin_v = exito_fin ? val_fin : atof(fs); } } const char *paso_ptr = strstr(ct, "PASO"); if (paso_ptr) { char ps_s[MAX_LINEA]; int pl = (ct + l) - (paso_ptr + 4); if (pl > 0 && pl < MAX_LINEA) { strncpy(ps_s, paso_ptr+4, pl); ps_s[pl]='\0'; limpiar_string(ps_s); int exito_paso; pas = evaluar_expresion_completa(ps_s, &exito_paso); if (!exito_paso) pas = atof(ps_s); if (pas == 0) pas = (fin_v >= 0) ? 1 : -1; } } } }
                int found = 0; double actual = 0;
                if (scope_actual >= 0) { ScopeLocal *scp = &scopes_locales[scope_actual]; for (int j = 0; j < scp->num_variables; j++) { if (strcmp(scp->variables[j].nombre, var_n) == 0) { switch(scp->variables[j].tipo) { case 0: actual = (double)scp->variables[j].valor.valor_entero; break; case 1: actual = (double)scp->variables[j].valor.valor_sin_signo; break; case 2: case 3: actual = scp->variables[j].valor.valor_decimal; break; case 4: actual = (double)scp->variables[j].valor.valor_caracter; break; case 5: actual = (double)scp->variables[j].valor.valor_caracter_sin_signo; break; } double nueva = actual + pas; switch(scp->variables[j].tipo) { case 0: scp->variables[j].valor.valor_entero = (int)nueva; break; case 1: scp->variables[j].valor.valor_sin_signo = (unsigned int)nueva; break; case 2: case 3: scp->variables[j].valor.valor_decimal = nueva; break; case 4: scp->variables[j].valor.valor_caracter = (char)nueva; break; case 5: scp->variables[j].valor.valor_caracter_sin_signo = (unsigned char)nueva; break; } found = 1; int cond = (pas > 0) ? (nueva <= fin_v) : (nueva >= fin_v); idx = cond ? (para_idx + 1) : (idx + 1); break; } } } if (!found) idx++;
            } else { idx++; }
            continue;
        }

        if (comienza_con(linea, "SEGUN CASO")) {
            double target_val = 0; const char *a = strchr(linea, '('); const char *c = strrchr(linea, ')');
            if (a && c && c > a + 1) { char expr[MAX_LINEA]; int len = c - a - 1; if (len < MAX_LINEA-1) { strncpy(expr, a+1, len); expr[len]='\0'; limpiar_string(expr); int ex; target_val = evaluar_expresion_completa(expr, &ex); if (!ex) target_val = 0; } }
            int found_case = 0, default_line = -1;
            for (int b = idx + 1; b <= linea_fin; b++) { char lb[MAX_LINEA]; strncpy(lb, lineas_programa[b], MAX_LINEA-1); lb[MAX_LINEA-1]='\0'; limpiar_string(lb); remover_comentario(lb); if (comienza_con(lb, "CASO")) { const char *p = lb + 4; while (*p == ' ' || *p == '\t') p++; double case_val = atof(p); if (case_val == target_val) { idx = b + 1; found_case = 1; break; } } else if (comienza_con(lb, "POR DEFECTO")) { if (default_line == -1) default_line = b; } else if (comienza_con(lb, "FIN SEGUN")) break; }
            if (!found_case) { if (default_line != -1) idx = default_line + 1; else { for (int b = idx + 1; b <= linea_fin; b++) { char lb[MAX_LINEA]; strncpy(lb, lineas_programa[b], MAX_LINEA-1); lb[MAX_LINEA-1]='\0'; limpiar_string(lb); remover_comentario(lb); if (comienza_con(lb, "FIN SEGUN")) { idx = b + 1; break; } } } }
            continue;
        }

        if (comienza_con(linea, "CORTE")) { for (int b = idx + 1; b <= linea_fin; b++) { char lb[MAX_LINEA]; strncpy(lb, lineas_programa[b], MAX_LINEA-1); lb[MAX_LINEA-1]='\0'; limpiar_string(lb); remover_comentario(lb); if (comienza_con(lb, "FIN SEGUN")) { idx = b + 1; break; } } continue; }
        if (comienza_con(linea, "CASO") || comienza_con(linea, "POR DEFECTO") || comienza_con(linea, "FIN SEGUN")) { idx++; continue; }
        if (strncmp(linea, "LLAMAR A ", 9) == 0 || strncmp(linea, "LLAMAR A(", 9) == 0) {
            const char *ptr = linea + 9; while (*ptr == ' ' || *ptr == '\t') ptr++; char nombre_func[MAX_NOMBRE]; int j = 0; while (*ptr && *ptr != '(' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1) nombre_func[j++] = *ptr++; nombre_func[j] = '\0';
            if (*ptr == '(') { char *args[MAX_PARAMETROS]; int num_args = 0; ptr++; while (*ptr && *ptr != ')' && num_args < MAX_PARAMETROS) { while (*ptr == ' ' || *ptr == '\t') ptr++; char arg[MAX_LINEA] = ""; int k = 0; int nivel = 0; while (*ptr && (nivel > 0 || (*ptr != ',' && *ptr != ')')) && k < MAX_LINEA - 1) { if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--; arg[k++] = *ptr++; } arg[k] = '\0'; limpiar_string(arg); if (strlen(arg) > 0) args[num_args++] = strdup(arg); if (*ptr == ',') ptr++; } int exito_call = 0; llamar_funcion(nombre_func, args, num_args, &exito_call); for (int k = 0; k < num_args; k++) free(args[k]); } idx++; continue;
        }

        idx++;
    }

    if (si_structural_ptr != 0) {
        fprintf(stderr, "Error estructural: SI en línea %d sin 'FIN SI'.\n", idx); 
        exit(1);
    }

    if (si_ejecucion_ptr != 0) {
        fprintf(stderr, "Error línea %d: Falta 'FIN SI' para cerrar este SI.\n", 
                si_ejecucion[si_ejecucion_ptr - 1].linea_apertura + 1);
        exit(1);
    }
    
    double valor_final = valor_retorno_funcion;
    strncpy(funcion_variable_destino, destino_anterior, MAX_NOMBRE - 1);
    funcion_variable_destino[MAX_NOMBRE - 1] = '\0';
    hay_valor_retorno = retorno_anterior;
    valor_retorno_funcion = valor_anterior;
    funcion_destino_stack_ptr = stack_ptr_anterior;
    if (funcion_destino_stack_ptr > 0 && funcion_destino_stack_ptr <= MAX_NESTING) {
        strncpy(funcion_variable_destino, funcion_destino_stack[funcion_destino_stack_ptr - 1], MAX_NOMBRE - 1);
        funcion_variable_destino[MAX_NOMBRE - 1] = '\0';
    } else {
        funcion_variable_destino[0] = '\0';
    }
    eliminar_scope_local();
    if (scope_actual < -1) scope_actual = -1;
    if (funcion_stack_ptr > 0) funcion_stack_ptr--;
    profundidad_funcion--;
    en_funcion = (profundidad_funcion > 0);

    *exito = 1;
    return valor_final;
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
