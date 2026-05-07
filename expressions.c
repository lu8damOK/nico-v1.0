/*
 * Nico v1.0.1 - Intérprete Educativo de Scripting en Español
 * @file:         expressions.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Evaluación recursiva de expresiones matemáticas, 
 *                lógicas (Y/O con cortocircuito) y funciones built-in.
 */
#include "nico.h"
#include <stdint.h> 
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern double evaluar_expresion_completa(const char *expr, int *exito);

// OBTENER VALOR DE TOKEN
double obtener_valor_token(const char *token, int *exito) {
    *exito = 0;
    if (!token || !strlen(token)) return 0;
    char token_limpio[MAX_LINEA];
    strncpy(token_limpio, token, MAX_LINEA - 1);
    token_limpio[MAX_LINEA - 1] = '\0';
    limpiar_string(token_limpio);
    
    // DETECTAR LLAMADA A FUNCIÓN
    char *parentesis = strchr(token_limpio, '(');
    if (parentesis) {
        char nombre_func[MAX_NOMBRE];
        int i = 0;
        char *p = token_limpio;
        while (*p && *p != '(' && i < MAX_NOMBRE - 1) {
            if (*p == '$') p++;
            else nombre_func[i++] = *p++;
        }
        nombre_func[i] = '\0';
        limpiar_string(nombre_func);
        
        char *args[MAX_PARAMETROS];
        int num_args = 0;
        
        p = parentesis + 1;
        while (*p && *p != ')' && num_args < MAX_PARAMETROS) {
            while (*p == ' ' || *p == '\t') p++;
            char arg[MAX_LINEA] = "";
            i = 0;
            int nivel_parentesis = 0;
            while (*p && (nivel_parentesis > 0 || (*p != ',' && *p != ')')) && i < MAX_LINEA - 1) {
                if (*p == '(') nivel_parentesis++;
                else if (*p == ')') nivel_parentesis--;
                arg[i++] = *p++;
            }
            arg[i] = '\0';
            limpiar_string(arg);
            if (strlen(arg) > 0) args[num_args++] = strdup(arg);
            if (*p == ',') p++;
        }
        
        int func_idx = buscar_funcion_info(nombre_func);
        if (func_idx >= 0) {
            int exito_func;
            double valor_retorno = llamar_funcion(nombre_func, args, num_args, &exito_func);
            for (int k = 0; k < num_args; k++) free(args[k]);
            if (exito_func) { *exito = 1; return valor_retorno; }
        }
        
        int exito_cadena = 0;
        double resultado_cadena = ejecutar_funcion_cadena(nombre_func, args, num_args, &exito_cadena);
        if (exito_cadena) {
            *exito = 1;
            return resultado_cadena;
        }
        
        for (int k = 0; k < num_args; k++) free(args[k]);
    }

    // DETECTAR ACCESO A MATRIZ
    char *corchete1 = strchr(token_limpio, '[');
    if (corchete1 && token_limpio[0] == '$') {
        char *cierre1 = strchr(corchete1 + 1, ']');
        char *corchete2 = NULL;
        if (cierre1) corchete2 = strchr(cierre1 + 1, '[');
    
            if (corchete2) {
                char nombre_matriz[MAX_NOMBRE];
                char fila_str[MAX_LINEA], columna_str[MAX_LINEA];
                int i = 0;
                char *p = token_limpio + 1;
                while (*p && *p != '[' && i < MAX_NOMBRE - 1) nombre_matriz[i++] = *p++;
                nombre_matriz[i] = '\0';
                limpiar_string(nombre_matriz);
        
                p = corchete1 + 1; i = 0; int nivel = 1;
                while (*p && nivel > 0 && i < MAX_LINEA - 1) {
                    if (*p == '[') nivel++; else if (*p == ']') nivel--;
                    if (nivel > 0) { fila_str[i++] = *p; } p++;
                }
                fila_str[i] = '\0'; limpiar_string(fila_str);
        
                p = corchete2 + 1; i = 0; nivel = 1;
                while (*p && nivel > 0 && i < MAX_LINEA - 1) {
                    if (*p == '[') nivel++; else if (*p == ']') nivel--;
                    if (nivel > 0) { columna_str[i++] = *p; } p++;
                }
                columna_str[i] = '\0'; limpiar_string(columna_str);
        
                int exito_fila, exito_columna;
                double val_fila = evaluar_expresion_completa(fila_str, &exito_fila);
                double val_columna = evaluar_expresion_completa(columna_str, &exito_columna);
                int fila = exito_fila ? (int)val_fila : atoi(fila_str);
                int columna = exito_columna ? (int)val_columna : atoi(columna_str);
        
                int tipo_mat_local = -1, idx_mat_local = -1;
                if (scope_actual >= 0 && buscar_matriz_local(nombre_matriz, &tipo_mat_local, &idx_mat_local)) {
                    switch(tipo_mat_local) {
                        case 0: *exito = 1; return (double)matrices_enteras[idx_mat_local].valores[fila][columna];
                        case 1: case 3: *exito = 1; return matrices_decimales[idx_mat_local].valores[fila][columna];
                        case 2: *exito = 1; return (double)matrices_enteras_sin_signo[idx_mat_local].valores[fila][columna];
                        case 4: *exito = 1; return (double)(unsigned char)matrices_caracter[idx_mat_local].valores[fila][columna];
                        case 5: *exito = 1; return (double)(unsigned char)matrices_caracter_sin_signo[idx_mat_local].valores[fila][columna];
                    }
                }
        
                if (scope_actual >= 0) {
                for (int s = scope_actual; s >= 0; s--) {
                    for (int v = 0; v < scopes_locales[s].num_variables; v++) {
                        if (strcmp(scopes_locales[s].variables[v].nombre, nombre_matriz) == 0) {
                            double val_param = 0;
                            switch(scopes_locales[s].variables[v].tipo) {
                                case 0: val_param = (double)scopes_locales[s].variables[v].valor.valor_entero; break;
                                case 1: val_param = (double)scopes_locales[s].variables[v].valor.valor_sin_signo; break;
                                case 2: case 3: val_param = scopes_locales[s].variables[v].valor.valor_decimal; break;
                                case 4: val_param = (double)scopes_locales[s].variables[v].valor.valor_caracter; break;
                                case 5: val_param = (double)scopes_locales[s].variables[v].valor.valor_caracter_sin_signo; break;
                                default: val_param = 0; break;
                            }
                        
                            if (val_param >= 2000000.0 && val_param < 2060000.0) {
                                int ref = (int)(val_param - 2000000.0);
                                int tipo_mat = ref / 10000;
                                int idx_pool = ref % 10000;
                            
                                switch(tipo_mat) {
                                    case 0: *exito = 1; return (double)matrices_enteras[idx_pool].valores[fila][columna];
                                    case 1: case 3: *exito = 1; return matrices_decimales[idx_pool].valores[fila][columna];
                                    case 2: *exito = 1; return (double)matrices_enteras_sin_signo[idx_pool].valores[fila][columna];
                                    case 4: *exito = 1; return (double)(unsigned char)matrices_caracter[idx_pool].valores[fila][columna];
                                    case 5: *exito = 1; return (double)(unsigned char)matrices_caracter_sin_signo[idx_pool].valores[fila][columna];
                                }
                            }
                            break; 
                        }
                    }
                }
             }
        
            int idx_matriz = -1;
            idx_matriz = buscar_matriz_entera(nombre_matriz);
            if (idx_matriz >= 0) { *exito = 1; return (double)get_matriz_entera_valor(nombre_matriz, fila, columna); }
            idx_matriz = buscar_matriz_decimal(nombre_matriz);
            if (idx_matriz >= 0) { *exito = 1; return get_matriz_decimal_valor(nombre_matriz, fila, columna); }
            idx_matriz = buscar_matriz_entera_sin_signo(nombre_matriz);
            if (idx_matriz >= 0) { *exito = 1; return (double)get_matriz_entera_sin_signo_valor(nombre_matriz, fila, columna); }
            idx_matriz = buscar_matriz_decimal_sin_signo(nombre_matriz);
            if (idx_matriz >= 0) { *exito = 1; return get_matriz_decimal_sin_signo_valor(nombre_matriz, fila, columna); }
            idx_matriz = buscar_matriz_caracter(nombre_matriz);
            if (idx_matriz >= 0) { *exito = 1; return (double)(unsigned char)get_matriz_caracter_valor(nombre_matriz, fila, columna); }
            idx_matriz = buscar_matriz_caracter_sin_signo(nombre_matriz);
            if (idx_matriz >= 0) { *exito = 1; return (double)(unsigned char)get_matriz_caracter_sin_signo_valor(nombre_matriz, fila, columna); }
        
            fprintf(stderr, "Error: Matriz '$%s' no declarada.\n", nombre_matriz);
            return 0;
        }
    }  
    // DETECTAR ACCESO A LISTA ($lista[indice])
    char *corchete = strchr(token_limpio, '[');
    if (corchete && token_limpio[0] == '$') {
        char nombre_lista[MAX_NOMBRE];
        char indice_str[MAX_LINEA];
        int i = 0;
        char *p = token_limpio + 1;
        while (*p && *p != '[' && i < MAX_NOMBRE - 1) nombre_lista[i++] = *p++;
        nombre_lista[i] = '\0';
        p++; i = 0;
        int nivel_corchete = 1;
        while (*p && nivel_corchete > 0 && i < MAX_LINEA - 1) {
            if (*p == '[') nivel_corchete++;
            else if (*p == ']') nivel_corchete--;
            if (nivel_corchete > 0) indice_str[i++] = *p;
            p++;
        }
        indice_str[i] = '\0';
        limpiar_string(indice_str);
        
        int exito_indice;
        double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
        int indice = exito_indice ? (int)val_indice : (es_numero(indice_str) ? atoi(indice_str) : 0);
       
        double val_param = 0;
        if (scope_actual >= 0 && scope_actual < MAX_SCOPES) {
            for (int v = 0; v < scopes_locales[scope_actual].num_variables; v++) {
                if (strcmp(scopes_locales[scope_actual].variables[v].nombre, nombre_lista) == 0) {
                    switch(scopes_locales[scope_actual].variables[v].tipo) {
                        case 0: val_param = (double)scopes_locales[scope_actual].variables[v].valor.valor_entero; break;
                        case 1: val_param = (double)scopes_locales[scope_actual].variables[v].valor.valor_sin_signo; break;
                        case 2: case 3: val_param = scopes_locales[scope_actual].variables[v].valor.valor_decimal; break;
                        case 4: val_param = (double)scopes_locales[scope_actual].variables[v].valor.valor_caracter; break;
                        case 5: val_param = (double)scopes_locales[scope_actual].variables[v].valor.valor_caracter_sin_signo; break;
                    }
                    if (val_param >= 1000000.0) {
                        int ref = (int)(val_param - 1000000.0);
                        int tipo_lista = ref / 10000;
                        int idx_pool = ref % 10000;
                        if (indice < 0 || indice > 63) { *exito = 1; return 0.0; }
                        switch(tipo_lista) {
                            case 0: *exito = 1; return (double)listas_enteras[idx_pool].valores[indice];
                            case 1: case 3: *exito = 1; return listas_decimales[idx_pool].valores[indice];
                            case 2: *exito = 1; return (double)listas_enteras_sin_signo[idx_pool].valores[indice];
                            case 4: *exito = 1; return (double)(unsigned char)listas_caracter[idx_pool].valores[indice];
                            case 5: *exito = 1; return (double)listas_caracter_sin_signo[idx_pool].valores[indice];
                        }
                    }
                    break;
                }
            }
        }        
        
        int tipo_decodificado = -1, idx_decodificado = -1;
        int es_referencia_codificada = 0;
        
        if (scope_actual >= 0) {
            for (int s = scope_actual; s >= 0; s--) {
                ScopeLocal *scp = &scopes_locales[s];
                for (int v = 0; v < scp->num_variables; v++) {
                    if (strcmp(scp->variables[v].nombre, nombre_lista) == 0) {
                        double val_param = 0;
                        switch(scp->variables[v].tipo) {
                            case 0: val_param = (double)scp->variables[v].valor.valor_entero; break;
                            case 1: val_param = (double)scp->variables[v].valor.valor_sin_signo; break;
                            case 2: case 3: val_param = scp->variables[v].valor.valor_decimal; break;
                            case 4: val_param = (double)scp->variables[v].valor.valor_caracter; break;
                            case 5: val_param = (double)scp->variables[v].valor.valor_caracter_sin_signo; break;
                        }
                        if (val_param >= 1000000.0 && val_param < 1010000.0) {
                            int ref = (int)(val_param - 1000000.0);
                            tipo_decodificado = ref / 10000;
                            idx_decodificado = ref % 10000;
                            es_referencia_codificada = 1;
                        }
                        break;
                    }
                }
                if (es_referencia_codificada) break;
            }
        }
        
        if (es_referencia_codificada) {
            switch(tipo_decodificado) {
                case 0: *exito = 1; return (double)listas_enteras[idx_decodificado].valores[indice];
                case 1: case 3: *exito = 1; return listas_decimales[idx_decodificado].valores[indice];
                case 2: *exito = 1; return (double)listas_enteras_sin_signo[idx_decodificado].valores[indice];
                case 4: *exito = 1; return (double)(unsigned char)listas_caracter[idx_decodificado].valores[indice];
                case 5: *exito = 1; return (double)listas_caracter_sin_signo[idx_decodificado].valores[indice];
                default: break;
            }
        }
        
        int tipo_local = -1, idx_local = -1;
        if (scope_actual >= 0 && buscar_lista_local(nombre_lista, &tipo_local, &idx_local)) {
            switch(tipo_local) {
                case 0: *exito = 1; return (double)listas_enteras[idx_local].valores[indice];
                case 1: case 3: *exito = 1; return listas_decimales[idx_local].valores[indice];
                case 2: *exito = 1; return (double)listas_enteras_sin_signo[idx_local].valores[indice];
                case 4: *exito = 1; return (double)(unsigned char)listas_caracter[idx_local].valores[indice];
                case 5: *exito = 1; return (double)listas_caracter_sin_signo[idx_local].valores[indice];
                default: break;
            }
        }

        if (scope_actual >= 0 && scope_actual < MAX_SCOPES) {
            int tipo_var;
            double val_var;
            if (buscar_variable_local(nombre_lista, &tipo_var, &val_var)) {
                if (val_var >= 1000000.0 && val_var < 1010000.0) {
                    int ref = (int)(val_var - 1000000.0);
                    int tipo_lista = ref / 10000;
                    int idx_pool = ref % 10000;
                    switch(tipo_lista) {
                        case 0: *exito = 1; return (double)listas_enteras[idx_pool].valores[indice];
                        case 1: case 3: *exito = 1; return listas_decimales[idx_pool].valores[indice];
                        case 2: *exito = 1; return (double)listas_enteras_sin_signo[idx_pool].valores[indice];
                        case 4: *exito = 1; return (double)(unsigned char)listas_caracter[idx_pool].valores[indice];
                        case 5: *exito = 1; return (double)listas_caracter_sin_signo[idx_pool].valores[indice];
                    }
                }
            }
        }

        int idx_lista = -1;
        if ((idx_lista = buscar_lista_entera(nombre_lista)) >= 0) { *exito = 1; return (double)get_lista_entera_valor(nombre_lista, indice); }
        if ((idx_lista = buscar_lista_decimal(nombre_lista)) >= 0) { *exito = 1; return get_lista_decimal_valor(nombre_lista, indice); }
        if ((idx_lista = buscar_lista_entera_sin_signo(nombre_lista)) >= 0) { *exito = 1; return (double)get_lista_entera_sin_signo_valor(nombre_lista, indice); }
        if ((idx_lista = buscar_lista_decimal_sin_signo(nombre_lista)) >= 0) { *exito = 1; return get_lista_decimal_sin_signo_valor(nombre_lista, indice); }
        if ((idx_lista = buscar_lista_caracter(nombre_lista)) >= 0) { *exito = 1; return (double)(unsigned char)get_lista_caracter_valor(nombre_lista, indice); }
        if ((idx_lista = buscar_lista_caracter_sin_signo(nombre_lista)) >= 0) { *exito = 1; return (double)(unsigned char)get_lista_caracter_sin_signo_valor(nombre_lista, indice); }
        
        fprintf(stderr, "Error: Lísta '$%s' no declarada.\n", nombre_lista);
        return 0; 
    }
    
    // CARACTER LITERAL
    if (token_limpio[0] == '\'' && strlen(token_limpio) >= 3) {
        *exito = 1;
        return (double)(unsigned char)token_limpio[1];
    }
    
    // NÚMERO HEXADECIMAL
    if (strlen(token_limpio) > 2 && token_limpio[0] == '0' &&
        (token_limpio[1] == 'x' || token_limpio[1] == 'X')) {
        *exito = 1;
        return (double)strtoul(token_limpio + 2, NULL, 16);
    }
    
    // NÚMERO OCTAL
    if (strlen(token_limpio) > 1 && token_limpio[0] == '0' &&
        token_limpio[1] >= '0' && token_limpio[1] <= '7') {
        *exito = 1;
        return (double)strtoul(token_limpio, NULL, 8);
    }
    
    // NÚMERO LITERAL
    if (es_numero(token_limpio) || (token_limpio[0] == '-' && es_numero(token_limpio + 1))) {
        *exito = 1;
        return atof(token_limpio);
    }
    
    // VARIABLE ($nombre sin corchetes)
    if (token_limpio[0] == '$') {
        char nombre[MAX_NOMBRE];
        limpiar_nombre(token_limpio, nombre, MAX_NOMBRE);
        
        int tipo_local;
        double valor_local;
        if (buscar_variable_local(nombre, &tipo_local, &valor_local)) {
            *exito = 1;
            return valor_local;
        }
        
        int idx = buscar_variable(nombre);
        if (idx >= 0) { *exito = 1; return (double)variables[idx].valor; }
        idx = buscar_variable_sin_signo(nombre);
        if (idx >= 0) { *exito = 1; return (double)variables_sin_signo[idx].valor; }
        idx = buscar_variable_decimal(nombre);
        if (idx >= 0) { *exito = 1; return variables_decimal[idx].valor; }
        idx = buscar_variable_decimal_sin_signo(nombre);
        if (idx >= 0) { *exito = 1; return variables_decimal_sin_signo[idx].valor; }
        idx = buscar_variable_caracter(nombre);
        if (idx >= 0) { *exito = 1; return (double)(unsigned char)variables_caracter[idx].valor; }
        idx = buscar_variable_caracter_sin_signo(nombre);
        if (idx >= 0) { *exito = 1; return (double)(unsigned char)variables_caracter_sin_signo[idx].valor; }
        
        idx = buscar_constante(nombre);
        if (idx >= 0) { *exito = 1; return (double)constantes[idx].valor; }
        idx = buscar_constante_sin_signo(nombre);
        if (idx >= 0) { *exito = 1; return (double)constantes_sin_signo[idx].valor; }
        idx = buscar_constante_decimal(nombre);
        if (idx >= 0) { *exito = 1; return constantes_decimal[idx].valor; }
        idx = buscar_constante_decimal_sin_signo(nombre);
        if (idx >= 0) { *exito = 1; return constantes_decimal_sin_signo[idx].valor; }
        idx = buscar_constante_caracter(nombre);
        if (idx >= 0) { *exito = 1; return (double)(unsigned char)constantes_caracter[idx].valor; }
        idx = buscar_constante_caracter_sin_signo(nombre);
        if (idx >= 0) { *exito = 1; return (double)(unsigned char)constantes_caracter_sin_signo[idx].valor; }
        
        idx = buscar_texto_constante(nombre);
        if (idx >= 0) { *exito = 1; return (double)strlen(texto_constantes[idx].valor); }
        idx = buscar_texto_var(nombre);
        if (idx >= 0) { *exito = 1; return (double)strlen(texto_vars[idx].valor); }
        
        int g = -1;
        if ((g = buscar_lista_entera(nombre)) >= 0) { *exito = 1; return 1000000.0 + (0 * 10000.0) + g; }
        if ((g = buscar_lista_decimal(nombre)) >= 0) { *exito = 1; return 1000000.0 + (1 * 10000.0) + g; }
        if ((g = buscar_lista_entera_sin_signo(nombre)) >= 0) { *exito = 1; return 1000000.0 + (2 * 10000.0) + g; }
        if ((g = buscar_lista_decimal_sin_signo(nombre)) >= 0) { *exito = 1; return 1000000.0 + (3 * 10000.0) + g; }
        if ((g = buscar_lista_caracter(nombre)) >= 0) { *exito = 1; return 1000000.0 + (4 * 10000.0) + g; }
        if ((g = buscar_lista_caracter_sin_signo(nombre)) >= 0) { *exito = 1; return 1000000.0 + (5 * 10000.0) + g; }

        return 0;
    }
    
    // BUSCAR SIN $
    int tipo_local_fallback;
    double valor_local_fallback;
    if (buscar_variable_local(token_limpio, &tipo_local_fallback, &valor_local_fallback)) {
        *exito = 1;
        return valor_local_fallback;
    }
    
    int idx_fb;
    idx_fb = buscar_variable(token_limpio);
    if (idx_fb >= 0) { *exito = 1; return (double)variables[idx_fb].valor; }
    idx_fb = buscar_variable_decimal(token_limpio);
    if (idx_fb >= 0) { *exito = 1; return variables_decimal[idx_fb].valor; }
    idx_fb = buscar_constante(token_limpio);
    if (idx_fb >= 0) { *exito = 1; return (double)constantes[idx_fb].valor; }
    idx_fb = buscar_constante_decimal(token_limpio);
    if (idx_fb >= 0) { *exito = 1; return constantes_decimal[idx_fb].valor; }
    return 0;
} 

// BUSCAR PALABRA CLAVE
char* buscar_palabra_clave(char *str, const char *palabra) {
    char *pos = strstr(str, palabra);
    while (pos) {
        int antes_ok = (pos == str) || (*(pos - 1) == ' ') || (*(pos - 1) == '(');
        int despues_ok = (*(pos + strlen(palabra)) == ' ') ||
                        (*(pos + strlen(palabra)) == ')') ||
                        (*(pos + strlen(palabra)) == '\0');
        if (antes_ok && despues_ok) return pos;
        pos = strstr(pos + 1, palabra);
    }
    return NULL;
}

// Elimina paréntesis externos si envuelven TODA la subcadena
static void limpiar_parentesis_externos(char *s) {
    if (!s) return;
    limpiar_string(s);
    int len = strlen(s);
    if (len < 2 || s[0] != '(' || s[len-1] != ')') return;
    
    int bal = 0;
    int es_externo = 1;
    for (int i = 0; i < len; i++) {
        if (s[i] == '(') bal++;
        else if (s[i] == ')') bal--;
        if (bal == 0 && i < len - 1) { es_externo = 0; break; }
    }
    
    if (es_externo) {
        memmove(s, s + 1, len - 2);
        s[len - 2] = '\0';
        limpiar_string(s);
    }
}

// EVALUAR CONDICIÓN SIMPLE (comparaciones)
int evaluar_condicion_simple(const char *condicion, int *exito) {
    *exito = 0;
    char cond_copy[MAX_LINEA];
    strncpy(cond_copy, condicion, MAX_LINEA - 1);
    cond_copy[MAX_LINEA - 1] = '\0';
    limpiar_string(cond_copy);

    char *partes, *izq, *der;
    int e_izq, e_der;
    double v_izq, v_der;

    const char* ops[] = {"MAYOR IGUAL", "MENOR IGUAL", "DIFERENTE", "IGUAL", "MAYOR", "MENOR"};
    int op_lens[] = {11, 11, 9, 5, 5, 5};

    for (int i = 0; i < 6; i++) {
        partes = strstr(cond_copy, ops[i]);
        if (partes) {
            *partes = '\0';
            der = partes + op_lens[i];
            izq = cond_copy;
            limpiar_parentesis_externos(izq);
            limpiar_parentesis_externos(der);
            
            v_izq = obtener_valor_token(izq, &e_izq);
            v_der = evaluar_expresion_completa(der, &e_der);
            
            if (e_izq && e_der) {
                *exito = 1;
                switch(i) {
                    case 0: return v_izq >= v_der;
                    case 1: return v_izq <= v_der;
                    case 2: return v_izq != v_der;
                    case 3: return v_izq == v_der;
                    case 4: return v_izq > v_der;
                    case 5: return v_izq < v_der;
                }
            }
            if (!e_izq) fprintf(stderr, "No se pudo evaluar el lado IZQUIERDO de '%s'. Verificá variables, funciones o sintaxis.\n", ops[i]);
            if (!e_der) fprintf(stderr, "No se pudo evaluar el lado DERECHO de '%s'.\n", ops[i]);
            *exito = 0;
            return 0;
        }
    }

    int e_val;
    double val = obtener_valor_token(condicion, &e_val);
    if (e_val) { *exito = 1; return (val != 0); }
    fprintf(stderr, "Condición o expresión inválida: '%s'.\n", condicion);
    fprintf(stderr, "Verificá operadores (MENOR, MAYOR, IGUAL, MENOR IGUAL, MAYOR IGUAL, DIFERENTE, Y, O) y que las variables lleven '$'.\n");
    *exito = 0; return 0;
}

// EVALUAR CONDICIÓN (maneja O / Y + paréntesis anidados)
int evaluar_condicion(const char *linea, int *exito) {
    *exito = 0;
    char cond_copy[MAX_LINEA];
    strncpy(cond_copy, linea, MAX_LINEA - 1);
    cond_copy[MAX_LINEA - 1] = '\0';
    limpiar_string(cond_copy);
    limpiar_parentesis_externos(cond_copy);

    char *p_o = strstr(cond_copy, " O ");
    if (p_o) {
        *p_o = '\0';
        char *izq = cond_copy;
        char *der = p_o + 3;
        limpiar_parentesis_externos(izq);
        limpiar_parentesis_externos(der);
        
        int e1, r1 = evaluar_condicion(izq, &e1);
        int e2, r2 = evaluar_condicion(der, &e2);
        if (e1 && e2) { *exito = 1; return r1 || r2; }
        if (!e1) fprintf(stderr, "Falló subcondión IZQUIERDA del O: '%s'.\n", izq);
        if (!e2) fprintf(stderr, "Falló subcondición DERECHA del O: '%s'.\n", der);
        *exito = 0;
        return 0;
    }

    char *p_y = strstr(cond_copy, " Y ");
    if (p_y) {
        *p_y = '\0';
        char *izq = cond_copy;
        char *der = p_y + 3;
        limpiar_parentesis_externos(izq);
        limpiar_parentesis_externos(der);
        
        int e1, r1 = evaluar_condicion(izq, &e1);
        int e2, r2 = evaluar_condicion(der, &e2);
        if (e1 && e2) { *exito = 1; return r1 && r2; }
        if (!e1) fprintf(stderr, "Falló subcondión IZQUIERDA del Y: '%s'.\n", izq);
        if (!e2) fprintf(stderr, "Falló subcondición DERECHA del Y: '%s'.\n", der);
        *exito = 0;
        return 0;
    }

    return evaluar_condicion_simple(cond_copy, exito);
}

// WRAPPERS PARA OPERACIONES BINARIAS
double nico_bity(double a, double b) {
    if (a < 0) a = 0;
    else if (a > 4294967295.0) a = 4294967295.0;
    if (b < 0) b = 0;
    else if (b > 4294967295.0) b = 4294967295.0;
    return (double)nico_bit_y((unsigned int)a, (unsigned int)b);
}

double nico_bito(double a, double b) {
    if (a < 0) a = 0;
    else if (a > 4294967295.0) a = 4294967295.0;
    if (b < 0) b = 0;
    else if (b > 4294967295.0) b = 4294967295.0;
    return (double)nico_bit_o((unsigned int)a, (unsigned int)b);
}

double nico_bitxor(double a, double b) {
    if (a < 0) a = 0;
    else if (a > 4294967295.0) a = 4294967295.0;
    if (b < 0) b = 0;
    else if (b > 4294967295.0) b = 4294967295.0;
    return (double)nico_bit_xor((unsigned int)a, (unsigned int)b);
}

double nico_bitno(double a) {
    unsigned int resultado = ~(unsigned int)a;
    if (resultado & 0x80000000) {
        return -(double)(~resultado + 1);
    }
    return (double)resultado;
}

double nico_desplazarizquierda(double a, double b) {
    return (double)nico_desplazar_izquierda((unsigned int)a, (int)b);
}

double nico_desplazarderecha(double a, double b) {
    return (double)nico_desplazar_derecha((unsigned int)a, (int)b);
}

double nico_rotarizquierda(double a, double b) {
    return (double)nico_rotar_izquierda((unsigned int)a, (int)b);
}

double nico_rotarderecha(double a, double b) {
    return (double)nico_rotar_derecha((unsigned int)a, (int)b);
}

double nico_leerbit(double a, double b) {
    return (double)nico_leer_bit((unsigned int)a, (int)b);
}

double nico_activarbit(double a, double b) {
    return (double)nico_activar_bit((unsigned int)a, (int)b);
}

double nico_desactivarbit(double a, double b) {
    return (double)nico_desactivar_bit((unsigned int)a, (int)b);
}

double nico_invertirbytes(double a) {
    return (double)nico_invertir_bytes((unsigned int)a);
}

double nico_contarbits(double a) {
    return (double)nico_contar_bits((unsigned int)a);
}

// EVALUAR EXPRESION COMPLETA
double evaluar_expresion_completa(const char *expr, int *exito) {
    *exito = 0;
    if (!expr || !strlen(expr)) return 0;
#define MAX_PROFUNDIDAD_EVAL 1024
    static int profundidad_eval = 0;

    if (++profundidad_eval > MAX_PROFUNDIDAD_EVAL) {
        fprintf(stderr, "Error: Recursión demasiado profunda (nivel %d > %d).\n", 
                profundidad_eval, MAX_PROFUNDIDAD_EVAL);
        fprintf(stderr, "Sugerencia: Revisar llamadas anidadas o aumentar MAX_PROFUNDIDAD_EVAL.\n");
        profundidad_eval--;
        *exito = 0;
        return 0;
    }
    char expresion[MAX_LINEA];
    strncpy(expresion, expr, MAX_LINEA - 1);
    expresion[MAX_LINEA - 1] = '\0';
    limpiar_string(expresion);
    
    double resultado = 0;
    const char *ptr = expresion;
    while (*ptr == ' ') ptr++;
    
    int signo = 1;
    if (*ptr == '-') {
        signo = -1;
        ptr++;
        while (*ptr == ' ') ptr++;
    } else if (*ptr == '+') {
        ptr++;
        while (*ptr == ' ') ptr++;
    }
    
    // PROCESAR PRIMER TOKEN O FUNCION
    if (strncmp(ptr, "PI()", 4) == 0) {
        resultado = 3.14159265358979;
        ptr += 4;
    }
    else if (strncmp(ptr, "SENO(", 5) == 0) {
        ptr += 5;
        char arg[MAX_LINEA] = "";
        int i = 0;
        int nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_func;
        double valor = evaluar_expresion_completa(arg, &exito_func);
        if (exito_func) resultado = sin(valor);
        *exito = 1;
    }
    else if (strncmp(ptr, "COSENO(", 7) == 0) {
        ptr += 7;
        char arg[MAX_LINEA] = "";
        int i = 0;
        int nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_func;
        double valor = evaluar_expresion_completa(arg, &exito_func);
        if (exito_func) resultado = cos(valor);
        *exito = 1;
    }
    else if (strncmp(ptr, "ALEATORIO(", 10) == 0) {
        ptr += 10;
        static int semilla_aleatorio_inicializada = 0;
        if (!semilla_aleatorio_inicializada) {
            srand((unsigned int)time(NULL));
            semilla_aleatorio_inicializada = 1;
        }
        char arg1[MAX_LINEA] = "", arg2[MAX_LINEA] = "", arg3[MAX_LINEA] = "";
        int i = 0, nivel = 1, num_arg = 0;
        while (*ptr && nivel > 0 && num_arg < 3) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            else if (*ptr == ',' && nivel == 1) {
                if (num_arg == 0) arg1[i] = '\0';
                else if (num_arg == 1) arg2[i] = '\0';
                i = 0; num_arg++; ptr++; continue;
            }
            if (nivel > 0) {
                if (num_arg == 0) arg1[i++] = *ptr;
                else if (num_arg == 1) arg2[i++] = *ptr;
                else arg3[i++] = *ptr;
            }
            ptr++;
        }
        if (num_arg == 0) arg1[i] = '\0';
        else if (num_arg == 1) arg2[i] = '\0';
        else arg3[i] = '\0';
        limpiar_string(arg1); limpiar_string(arg2); limpiar_string(arg3);
        int exito_func;
        double minimo = evaluar_expresion_completa(arg2, &exito_func);
        double maximo = evaluar_expresion_completa(arg3, &exito_func);
        if (!exito_func) { *exito = 0; return 0.0; }
        double valor = (double)rand() / (double)RAND_MAX;
        double resultado_raw = minimo + valor * (maximo - minimo);
        if (strcmp(arg1, "DECIMAL") == 0) {
            resultado = resultado_raw;
        } else if (strcmp(arg1, "CARACTER") == 0) {
            int char_min = 65, char_max = 90;
            for (int k = 0; k < MAX_LINEA && arg2[k]; k++) {
                if (arg2[k] == '\'' && arg2[k+1]) { char_min = (unsigned char)arg2[k+1]; break; }
            }
            for (int k = 0; k < MAX_LINEA && arg3[k]; k++) {
                if (arg3[k] == '\'' && arg3[k+1]) { char_max = (unsigned char)arg3[k+1]; break; }
            }
            double val_char = (double)rand() / (double)RAND_MAX;
            resultado = (double)(int)(char_min + val_char * (char_max - char_min + 1));
        } else {
            resultado = (double)(int)resultado_raw;
        }
    }
    else if (strncmp(ptr, "ALEATORIOSINSIGNO(", 18) == 0) {
        ptr += 18;
        static int semilla_sin_signo_inicializada = 0;
        if (!semilla_sin_signo_inicializada) {
            srand((unsigned int)time(NULL));
            semilla_sin_signo_inicializada = 1;
        }
        char arg1[MAX_LINEA] = "", arg2[MAX_LINEA] = "", arg3[MAX_LINEA] = "";
        int i = 0, nivel = 1, num_arg = 0;
        while (*ptr && nivel > 0 && num_arg < 3) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            else if (*ptr == ',' && nivel == 1) {
                if (num_arg == 0) arg1[i] = '\0';
                else if (num_arg == 1) arg2[i] = '\0';
                i = 0; num_arg++; ptr++; continue;
            }
            if (nivel > 0) {
                if (num_arg == 0) arg1[i++] = *ptr;
                else if (num_arg == 1) arg2[i++] = *ptr;
                else arg3[i++] = *ptr;
            }
            ptr++;
        }
        if (num_arg == 0) arg1[i] = '\0';
        else if (num_arg == 1) arg2[i] = '\0';
        else arg3[i] = '\0';
        limpiar_string(arg1); limpiar_string(arg2); limpiar_string(arg3);
        int exito_func;
        double minimo = evaluar_expresion_completa(arg2, &exito_func);
        double maximo = evaluar_expresion_completa(arg3, &exito_func);
        if (!exito_func) { *exito = 0; return 0.0; }
        double valor = (double)rand() / (double)RAND_MAX;
        double resultado_raw = minimo + valor * (maximo - minimo);
        if (strcmp(arg1, "DECIMAL") == 0) {
            resultado = resultado_raw;
        } else {
            if (resultado_raw < 0) resultado_raw = -resultado_raw;
            resultado = (double)(unsigned int)resultado_raw;
        }
    }
    else if (strncmp(ptr, "TANGENTE(", 9) == 0) {
        ptr += 9;
        char arg[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_func;
        double valor = evaluar_expresion_completa(arg, &exito_func);
        if (exito_func) resultado = tan(valor);
        *exito = 1;
    }
    else if (strncmp(ptr, "RAIZ(", 5) == 0) {
        ptr += 5;
        char arg[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_func;
        double valor = evaluar_expresion_completa(arg, &exito_func);
        if (exito_func) resultado = sqrt(valor);
        *exito = 1;
    }
    else if (strncmp(ptr, "POTENCIA(", 9) == 0) {
        ptr += 9;
        char arg1[MAX_LINEA] = "", arg2[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && i < MAX_LINEA - 1) {
            if (*ptr == '(') { nivel++; arg1[i++] = *ptr++; }
            else if (*ptr == ')') { nivel--; if (nivel == 0) break; arg1[i++] = *ptr++; }
            else if (*ptr == ',' && nivel == 1) { ptr++; break; }
            else { arg1[i++] = *ptr++; }
        }
        arg1[i] = '\0'; limpiar_string(arg1);
        i = 0;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg2[i++] = *ptr;
            ptr++;
        }
        arg2[i] = '\0'; limpiar_string(arg2);
        int exito1, exito2;
        double base = evaluar_expresion_completa(arg1, &exito1);
        double exp = evaluar_expresion_completa(arg2, &exito2);
        if (exito1 && exito2) resultado = pow(base, exp);
        *exito = 1; 
    }
    else if (strncmp(ptr, "MODULO(", 7) == 0) {
        ptr += 7;
        char arg1[MAX_LINEA] = "", arg2[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && i < MAX_LINEA - 1) {
            if (*ptr == '(') { nivel++; arg1[i++] = *ptr++; }
            else if (*ptr == ')') { nivel--; if (nivel == 0) break; arg1[i++] = *ptr++; }
            else if (*ptr == ',' && nivel == 1) { ptr++; break; }
            else { arg1[i++] = *ptr++; }
        }
        arg1[i] = '\0'; limpiar_string(arg1);
        i = 0;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg2[i++] = *ptr;
            ptr++;
        }
        arg2[i] = '\0'; limpiar_string(arg2);
        int exito1, exito2;
        double a = evaluar_expresion_completa(arg1, &exito1);
        double b = evaluar_expresion_completa(arg2, &exito2);
        if (exito1 && exito2 && (int)b != 0) resultado = fmod(a, b);
        *exito = 1;
    }
    else if (strncmp(ptr, "MAXIMO(", 7) == 0) {
        ptr += 7;
        char arg1[MAX_LINEA] = "", arg2[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && i < MAX_LINEA - 1) {
            if (*ptr == '(') { nivel++; arg1[i++] = *ptr++; }
            else if (*ptr == ')') { nivel--; if (nivel == 0) break; arg1[i++] = *ptr++; }
            else if (*ptr == ',' && nivel == 1) { ptr++; break; }
            else { arg1[i++] = *ptr++; }
        }
        arg1[i] = '\0'; limpiar_string(arg1);
        i = 0;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg2[i++] = *ptr;
            ptr++;
        }
        arg2[i] = '\0'; limpiar_string(arg2);
        int exito1, exito2;
        double a = evaluar_expresion_completa(arg1, &exito1);
        double b = evaluar_expresion_completa(arg2, &exito2);
        if (exito1 && exito2) resultado = (a > b) ? a : b;
        *exito = 1;
    }
    else if (strncmp(ptr, "MINIMO(", 7) == 0) {
        ptr += 7;
        char arg1[MAX_LINEA] = "", arg2[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && i < MAX_LINEA - 1) {
            if (*ptr == '(') { nivel++; arg1[i++] = *ptr++; }
            else if (*ptr == ')') { nivel--; if (nivel == 0) break; arg1[i++] = *ptr++; }
            else if (*ptr == ',' && nivel == 1) { ptr++; break; }
            else { arg1[i++] = *ptr++; }
        }
        arg1[i] = '\0'; limpiar_string(arg1);
        i = 0;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg2[i++] = *ptr;
            ptr++;
        }
        arg2[i] = '\0'; limpiar_string(arg2);
        int exito1, exito2;
        double a = evaluar_expresion_completa(arg1, &exito1);
        double b = evaluar_expresion_completa(arg2, &exito2);
        if (exito1 && exito2) resultado = (a < b) ? a : b;
        *exito = 1;
    }
    else if (strncmp(ptr, "ABSOLUTO(", 9) == 0) {
        ptr += 9;
        char arg[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_func;
        double valor = evaluar_expresion_completa(arg, &exito_func);
        if (exito_func) resultado = fabs(valor);
        *exito = 1;
    }

    // FUNCIONES MATEMÁTICAS
    else if (strncmp(ptr, "NUMEROPI()", 10) == 0) {
        resultado = nico_numeropi(); ptr += 10;
    }

    else if (strncmp(ptr, "NUMEROEULER()", 13) == 0) {
        resultado = nico_numeroeuler(); ptr += 13;
    }

    else if (strncmp(ptr, "RAIZDEUNMEDIO()", 15) == 0) {
        resultado = nico_raizdeunmedio(); ptr += 15;
    }

    else if (strncmp(ptr, "LOGNATURALDE2()", 15) == 0) {
        resultado = nico_lognaturalde2(); ptr += 15;
    }

    else if (strncmp(ptr, "LOGNATURALDE10()", 16) == 0) {
        resultado = nico_lognaturalde10(); ptr += 16;
    }
    
    else if (strncmp(ptr, "RAIZCUBICA(", 11) == 0) {
        ptr += 11;
        char arg[MAX_LINEA] = ""; int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) {
            resultado = nico_raizcubica(val);
            *exito = 1;
        }
    }

    else if (strncmp(ptr, "ARCOSENO(", 9) == 0) {
        ptr += 9;
        char arg[MAX_LINEA] = ""; int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) { arg[i++] = *ptr; } ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) {
            resultado = nico_arcoseno(val);
            *exito = 1;
        }
    }

    else if (strncmp(ptr, "ARCOCOSENO(", 11) == 0) {
        ptr += 11;
        char arg[MAX_LINEA] = ""; int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) { arg[i++] = *ptr; } ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) {
            resultado = nico_arcocoseno(val);
            *exito = 1;
        }
    }

    else if (strncmp(ptr, "ARCOTANGENTE(", 13) == 0) {
        ptr += 13;
        char arg[MAX_LINEA] = ""; int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) { arg[i++] = *ptr; } ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) {
            resultado = nico_arcotangente(val);
            *exito = 1;
        }
    }

    else if (strncmp(ptr, "LOGNATURAL(", 11) == 0) {
        ptr += 11;
        char arg[MAX_LINEA] = ""; int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) { arg[i++] = *ptr; } ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) {
            resultado = nico_lognatural(val);
            *exito = 1;
        }
    }

    else if (strncmp(ptr, "LOGBASE10(", 10) == 0) {
        ptr += 10;
        char arg[MAX_LINEA] = ""; int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) { arg[i++] = *ptr; } ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) { 
            resultado = nico_logbase10(val);
            *exito = 1;
        }
    }

    else if (strncmp(ptr, "LOGBASE2(", 9) == 0) {
        ptr += 9;
        char arg[MAX_LINEA] = ""; int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) { arg[i++] = *ptr; } ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) { 
            resultado = nico_logbase2(val);
            *exito = 1;
        }
    }

    else if (strncmp(ptr, "LOGARITMO(", 10) == 0) {
        ptr += 10;
        char arg1[MAX_LINEA] = "", arg2[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && i < MAX_LINEA - 1) {
            if (*ptr == '(') { nivel++; arg1[i++] = *ptr++; }
            else if (*ptr == ')') { nivel--; if (nivel == 0) break; arg1[i++] = *ptr++; }
            else if (*ptr == ',' && nivel == 1) { ptr++; break; }
            else { arg1[i++] = *ptr++; }
        }
        arg1[i] = '\0'; limpiar_string(arg1);
        i = 0;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg2[i++] = *ptr;
            ptr++;
        }
        arg2[i] = '\0'; limpiar_string(arg2);
        
        int exito1, exito2;
        double num = evaluar_expresion_completa(arg1, &exito1);
        double base = evaluar_expresion_completa(arg2, &exito2);
        if (exito1 && exito2 && base > 0 && base != 1 && num > 0) {
            resultado = nico_logaritmo_base(num, base);
            *exito = 1;
        }
    }

    else if (strncmp(ptr, "EXPONENCIAL(", 12) == 0) {
        ptr += 12;
        char arg[MAX_LINEA] = ""; int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) { arg[i++] = *ptr; } ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) { 
            resultado = nico_exponencial(val);
            *exito = 1;
        }
    }

    else if (strncmp(ptr, "DOSALAX(", 8) == 0) {
        ptr += 8;
        char arg[MAX_LINEA] = ""; int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) { arg[i++] = *ptr; } ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) {
            resultado = nico_dosalax(val);
            *exito = 1;
        }
    }
    
    else if (strncmp(ptr, "REDONDEAR(", 10) == 0) {
        ptr += 10;
        char modo[16] = ""; int i = 0;
        while (*ptr && *ptr != ',' && *ptr != ')' && i < 15) modo[i++] = *ptr++;
        modo[i] = '\0'; limpiar_string(modo);
        
        while (*ptr == ' ' || *ptr == '\t' || *ptr == ',') ptr++;
        
        char arg[MAX_LINEA] = ""; i = 0; int nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        
        if (exito_arg) {
            if (strcmp(modo, "ARRIBA") == 0) resultado = nico_redondear_arriba(val);
            else if (strcmp(modo, "ABAJO") == 0) resultado = nico_redondear_abajo(val);
            else if (strcmp(modo, "ENTERO") == 0) resultado = nico_redondear_entero(val);
            else { fprintf(stderr, "Error: Modo de REDONDEAR no reconocido: '%s'.\n", modo); }
            *exito = 1;
        }
    }

    else if (strncmp(ptr, "QUITARDECIMAL(", 14) == 0) {
        ptr += 14;
        char arg[MAX_LINEA] = ""; int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++; else if (*ptr == ')') nivel--;
            if (nivel > 0) { arg[i++] = *ptr; } ptr++;
        }
        arg[i] = '\0';
        int exito_arg; double val = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg){ 
            resultado = nico_quitar_decimal(val);
            *exito = 1; 
        }
    }

    else if (strncmp(ptr, "SIGMOIDE(", 9) == 0) {
        ptr += 9;
        char arg[MAX_LINEA] = "";
        int i = 0;
        int nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_func;
        double valor = evaluar_expresion_completa(arg, &exito_func);
        if (exito_func) {
            resultado = 1.0 / (1.0 + exp(-valor));
        }
        *exito = 1;
    }

    else if (strncmp(ptr, "BITNO(", 6) == 0) {
        ptr += 6;
        char arg[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_arg;
        double valor_arg = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) { resultado = nico_bitno(valor_arg); *exito = 1;}
        else { fprintf(stderr, "Error: No se pudo evaluar argumento de BITNO.\n"); return 0; }
    }
    else if (strncmp(ptr, "INVERTIRBYTES(", 14) == 0) {
        ptr += 14;
        char arg[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_arg;
        double valor_arg = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) { resultado = nico_invertirbytes(valor_arg); *exito = 1;}
        else { fprintf(stderr, "Error: No se pudo evaluar argumento de INVERTIRBYTES.\n"); return 0; }
    }

    else if (strncmp(ptr, "CONTARBITS(", 11) == 0) {
        ptr += 11;
        char arg[MAX_LINEA] = "";
        int i = 0, nivel = 1;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) arg[i++] = *ptr;
            ptr++;
        }
        arg[i] = '\0';
        int exito_arg;
        double valor_arg = evaluar_expresion_completa(arg, &exito_arg);
        if (exito_arg) { resultado = nico_contarbits(valor_arg); *exito = 1; }
        else { fprintf(stderr, "Error: No se pudo evaluar argumento de CONTARBITS.\n"); return 0; }
    }

    else if (strncmp(ptr, "BITY(", 5) == 0) {
        ptr += 5;
        char a1[MAX_LINEA]="", a2[MAX_LINEA]=""; int i=0, n=1;
        while(*ptr && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')'){n--; if(n==0)break;} else if(*ptr==',' && n==1){ptr++; break;} else a1[i++]=*ptr++; } a1[i]='\0'; limpiar_string(a1);
        i=0; while(*ptr && n>0 && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')')n--; if(n>0) a2[i++]=*ptr; ptr++; } a2[i]='\0'; limpiar_string(a2);
        int e1,e2; double v1=evaluar_expresion_completa(a1,&e1), v2=evaluar_expresion_completa(a2,&e2);
        if(e1 && e2) { resultado = nico_bity(v1, v2); *exito = 1; }
    }

    else if (strncmp(ptr, "BITO(", 5) == 0) {
        ptr += 5;
        char a1[MAX_LINEA]="", a2[MAX_LINEA]=""; int i=0, n=1;
        while(*ptr && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')'){n--; if(n==0)break;} else if(*ptr==',' && n==1){ptr++; break;} else a1[i++]=*ptr++; } a1[i]='\0'; limpiar_string(a1);
        i=0; while(*ptr && n>0 && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')')n--; if(n>0) a2[i++]=*ptr; ptr++; } a2[i]='\0'; limpiar_string(a2);
        int e1,e2; double v1=evaluar_expresion_completa(a1,&e1), v2=evaluar_expresion_completa(a2,&e2);
        if(e1 && e2) { resultado = nico_bito(v1, v2); *exito = 1; }
    }

    // OPERADORES BITWISE
    else if (strncmp(ptr, "BITXOR(", 7) == 0) {
        ptr += 7;
        char a1[MAX_LINEA]="", a2[MAX_LINEA]=""; int i=0, n=1;
        while(*ptr && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')'){n--; if(n==0)break;} else if(*ptr==',' && n==1){ptr++; break;} else a1[i++]=*ptr++; } a1[i]='\0'; limpiar_string(a1);
        i=0; while(*ptr && n>0 && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')')n--; if(n>0) a2[i++]=*ptr; ptr++; } a2[i]='\0'; limpiar_string(a2);
        int e1,e2; double v1=evaluar_expresion_completa(a1,&e1), v2=evaluar_expresion_completa(a2,&e2);
        if(e1 && e2) { resultado = nico_bitxor(v1, v2); *exito = 1; }
    }

    else if (strncmp(ptr, "DESPLAZARIZQUIERDA(", 19) == 0) {
        ptr += 19;
        char a1[MAX_LINEA]="", a2[MAX_LINEA]=""; int i=0, n=1;
        while(*ptr && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')'){n--; if(n==0)break;} else if(*ptr==',' && n==1){ptr++; break;} else a1[i++]=*ptr++; } a1[i]='\0'; limpiar_string(a1);
        i=0; while(*ptr && n>0 && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')')n--; if(n>0) a2[i++]=*ptr; ptr++; } a2[i]='\0'; limpiar_string(a2);
        int e1,e2; double v1=evaluar_expresion_completa(a1,&e1), v2=evaluar_expresion_completa(a2,&e2);
        if(e1 && e2) { resultado = nico_desplazarizquierda(v1, v2); *exito = 1; }
    }

    else if (strncmp(ptr, "DESPLAZARDERECHA(", 17) == 0) {
        ptr += 17;
        char a1[MAX_LINEA]="", a2[MAX_LINEA]=""; int i=0, n=1;
        while(*ptr && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')'){n--; if(n==0)break;} else if(*ptr==',' && n==1){ptr++; break;} else a1[i++]=*ptr++; } a1[i]='\0'; limpiar_string(a1);
        i=0; while(*ptr && n>0 && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')')n--; if(n>0) a2[i++]=*ptr; ptr++; } a2[i]='\0'; limpiar_string(a2);
        int e1,e2; double v1=evaluar_expresion_completa(a1,&e1), v2=evaluar_expresion_completa(a2,&e2);
        if(e1 && e2) { resultado = nico_desplazarderecha(v1, v2); *exito = 1; }
    }

    else if (strncmp(ptr, "ROTARIZQUIERDA(", 15) == 0) {
        ptr += 15;
        char a1[MAX_LINEA]="", a2[MAX_LINEA]=""; int i=0, n=1;
        while(*ptr && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')'){n--; if(n==0)break;} else if(*ptr==',' && n==1){ptr++; break;} else a1[i++]=*ptr++; } a1[i]='\0'; limpiar_string(a1);
        i=0; while(*ptr && n>0 && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')')n--; if(n>0) a2[i++]=*ptr; ptr++; } a2[i]='\0'; limpiar_string(a2);
        int e1,e2; double v1=evaluar_expresion_completa(a1,&e1), v2=evaluar_expresion_completa(a2,&e2);
        if(e1 && e2) { resultado = nico_rotarizquierda(v1, v2); *exito = 1; }
    }

    else if (strncmp(ptr, "ROTARDERECHA(", 13) == 0) {
        ptr += 13;
        char a1[MAX_LINEA]="", a2[MAX_LINEA]=""; int i=0, n=1;
        while(*ptr && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')'){n--; if(n==0)break;} else if(*ptr==',' && n==1){ptr++; break;} else a1[i++]=*ptr++; } a1[i]='\0'; limpiar_string(a1);
        i=0; while(*ptr && n>0 && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')')n--; if(n>0) a2[i++]=*ptr; ptr++; } a2[i]='\0'; limpiar_string(a2);
        int e1,e2; double v1=evaluar_expresion_completa(a1,&e1), v2=evaluar_expresion_completa(a2,&e2);
        if(e1 && e2) { resultado = nico_rotarderecha(v1, v2); *exito = 1; }
    }

    else if (strncmp(ptr, "LEERBIT(", 8) == 0) {
        ptr += 8;
        char a1[MAX_LINEA]="", a2[MAX_LINEA]=""; int i=0, n=1;
        while(*ptr && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')'){n--; if(n==0)break;} else if(*ptr==',' && n==1){ptr++; break;} else a1[i++]=*ptr++; } a1[i]='\0'; limpiar_string(a1);
        i=0; while(*ptr && n>0 && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')')n--; if(n>0) a2[i++]=*ptr; ptr++; } a2[i]='\0'; limpiar_string(a2);
        int e1,e2; double v1=evaluar_expresion_completa(a1,&e1), v2=evaluar_expresion_completa(a2,&e2);
        if(e1 && e2) { resultado = nico_leerbit(v1, v2); *exito = 1; }
    }

    else if (strncmp(ptr, "ACTIVARBIT(", 11) == 0) {
        ptr += 11;
        char a1[MAX_LINEA]="", a2[MAX_LINEA]=""; int i=0, n=1;
        while(*ptr && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')'){n--; if(n==0)break;} else if(*ptr==',' && n==1){ptr++; break;} else a1[i++]=*ptr++; } a1[i]='\0'; limpiar_string(a1);
        i=0; while(*ptr && n>0 && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')')n--; if(n>0) a2[i++]=*ptr; ptr++; } a2[i]='\0'; limpiar_string(a2);
        int e1,e2; double v1=evaluar_expresion_completa(a1,&e1), v2=evaluar_expresion_completa(a2,&e2);
        if(e1 && e2) { resultado = nico_activarbit(v1, v2); *exito = 1; }
    }

    else if (strncmp(ptr, "DESACTIVARBIT(", 14) == 0) {
        ptr += 14;
        char a1[MAX_LINEA]="", a2[MAX_LINEA]=""; int i=0, n=1;
        while(*ptr && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')'){n--; if(n==0)break;} else if(*ptr==',' && n==1){ptr++; break;} else a1[i++]=*ptr++; } a1[i]='\0'; limpiar_string(a1);
        i=0; while(*ptr && n>0 && i<MAX_LINEA-1) { if(*ptr=='(')n++; else if(*ptr==')')n--; if(n>0) a2[i++]=*ptr; ptr++; } a2[i]='\0'; limpiar_string(a2);
        int e1,e2; double v1=evaluar_expresion_completa(a1,&e1), v2=evaluar_expresion_completa(a2,&e2);
        if(e1 && e2) { resultado = nico_desactivarbit(v1, v2); *exito = 1; }
    }

    else if (strncmp(ptr, "FINARCHIVO(", 11) == 0) {
        ptr += 11;
        char nombre_var[MAX_NOMBRE] = "";
        int i = 0;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        while (*ptr && *ptr != ')' && i < MAX_NOMBRE - 1) {
            if (*ptr == '$') ptr++;
            else nombre_var[i++] = *ptr++;
        }
        nombre_var[i] = '\0';
        for (int k = 0; k < MAX_VARS_ARCHIVO; k++) {
            if (variables_archivo[k].archivo != NULL && 
                strcmp(variables_archivo[k].nombre, nombre_var) == 0) {
                *exito = 1;
                return (double)feof(variables_archivo[k].archivo);
            }
        }
        *exito = 1;
        return 0.0;
    }
    else if (*ptr == '(') {
        ptr++;
        char subexpr[MAX_LINEA] = "";
        int nivel = 1, i = 0;
        while (*ptr && nivel > 0 && i < MAX_LINEA - 1) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            if (nivel > 0) subexpr[i++] = *ptr;
            ptr++;
        }
        subexpr[i] = '\0';
        int exito_sub = 0;
        resultado = evaluar_expresion_completa(subexpr, &exito_sub);
        if (!exito_sub) {
            fprintf(stderr, "Error: No se pudo evaluar subexpresion '%s'.\n", subexpr);
            profundidad_eval--;
            return 0;
        }
    }
    else {
        char token[MAX_LINEA];
        int i = 0;
        int nivel_corchete = 0;
        int nivel_parentesis_funcion = 0;
        while (*ptr && i < MAX_LINEA - 1) {
            if (*ptr == '[') { nivel_corchete++; token[i++] = *ptr++; }
            else if (*ptr == ']' && nivel_corchete > 0) { nivel_corchete--; token[i++] = *ptr++; }
            else if (*ptr == '(' && nivel_corchete == 0) {
                nivel_parentesis_funcion = 1;
                token[i++] = *ptr++;
                while (*ptr && nivel_parentesis_funcion > 0 && i < MAX_LINEA - 1) {
                    if (*ptr == '(') nivel_parentesis_funcion++;
                    else if (*ptr == ')') nivel_parentesis_funcion--;
                    token[i++] = *ptr++;
                }
            }
            else if (nivel_corchete == 0 && nivel_parentesis_funcion == 0 &&
                    (*ptr == ' ' || *ptr == '+' || *ptr == '-' || *ptr == '*' || *ptr == '/')) {
                break;
            }
            else { token[i++] = *ptr++; }
        }
        token[i] = '\0';
        limpiar_string(token);
        int exito_val;
        resultado = obtener_valor_token(token, &exito_val);
        if (!exito_val) {
            fprintf(stderr, "Error: No se pudo evaluar '%s'.\n", token);
            profundidad_eval--; 
            return 0;
        }
    }
    
    resultado *= signo;
    
    // PROCESAR OPERADORES ARITMÉTICOS BÁSICOS
    while (*ptr) {
        while (*ptr == ' ') ptr++;
        if (!*ptr) break;
        
        char op = *ptr;
        if (op != '+' && op != '-' && op != '*' && op != '/' && op != '^' && op != '%') break; 
        ptr++;
        while (*ptr == ' ') ptr++;
        if (!*ptr) break;
        
        double valor;
        char token[MAX_LINEA];
        int i = 0;
        int nivel_corchete = 0;
        int nivel_parentesis_funcion = 0;
        while (*ptr && i < MAX_LINEA - 1) {
            if (*ptr == '[') { nivel_corchete++; token[i++] = *ptr++; }
            else if (*ptr == ']' && nivel_corchete > 0) { nivel_corchete--; token[i++] = *ptr++; }
            else if (*ptr == '(' && nivel_corchete == 0) {
                nivel_parentesis_funcion = 1;
                token[i++] = *ptr++;
                while (*ptr && nivel_parentesis_funcion > 0 && i < MAX_LINEA - 1) {
                    if (*ptr == '(') nivel_parentesis_funcion++;
                    else if (*ptr == ')') nivel_parentesis_funcion--;
                    token[i++] = *ptr++;
                }
            }
            else if (nivel_corchete == 0 && nivel_parentesis_funcion == 0 &&
                (*ptr == ' ' || *ptr == '+' || *ptr == '-' || *ptr == '*' || *ptr == '/' || *ptr == '%')) {
                break;
            }
            else { token[i++] = *ptr++; }
        }
        token[i] = '\0';
        limpiar_string(token);
        
        int exito_val;
        valor = evaluar_expresion_completa(token, &exito_val);
        if (!exito_val) {
            fprintf(stderr, "Error: No se pudo evaluar '%s'.\n", token);
            return 0;
        }
        
        if (op == '+') resultado += valor;
        else if (op == '-') resultado -= valor;
        else if (op == '*') resultado *= valor;
        else if (op == '/') {
            if (valor == 0) { fprintf(stderr, "Error: División por cero.\n"); return 0; }
                resultado /= valor;
            }
        else if (op == '%') {
            if ((int)valor == 0) { fprintf(stderr, "Error: División por cero en módulo.\n"); return 0; }
                resultado = (int)resultado % (int)valor;
            if (resultado < 0) resultado += (int)valor;
        }
        else if (op == '^') { resultado = pow(resultado, valor); }
    }

    // PROCESAR OPERADORES BINARIOS
    const char *op_names[] = {
        "BITY", "BITO", "BITXOR", 
        "DESPLAZARIZQUIERDA", "DESPLAZARDERECHA",
        "ROTARIZQUIERDA", "ROTARDERECHA",
        "LEERBIT", "ACTIVARBIT", "DESACTIVARBIT",
        NULL
    };
    
    for (int op_idx = 0; op_names[op_idx] != NULL; op_idx++) {
        const char *op_name = op_names[op_idx];
        char search_pattern[64];
        snprintf(search_pattern, sizeof(search_pattern), " %s ", op_name);
        
        char *op_ptr = strstr(expresion, search_pattern);
        if (op_ptr) {
            char izq[MAX_LINEA] = "";
            int len_izq = op_ptr - expresion;
            if (len_izq > 0 && len_izq < MAX_LINEA) {
                strncpy(izq, expresion, len_izq);
                izq[len_izq] = '\0';
                limpiar_string(izq);
            }
            
            char der[MAX_LINEA] = "";
            char *der_start = op_ptr + strlen(search_pattern);
            strncpy(der, der_start, MAX_LINEA - 1);
            der[MAX_LINEA - 1] = '\0';
            limpiar_string(der);
            
            int exito_izq, exito_der;
            double val_izq = evaluar_expresion_completa(izq, &exito_izq);
            double val_der = evaluar_expresion_completa(der, &exito_der);
            
            if (exito_izq && exito_der) {
                double resultado_binario = 0;
                
                if (strcmp(op_name, "BITY") == 0) resultado_binario = nico_bity(val_izq, val_der);
                else if (strcmp(op_name, "BITO") == 0) resultado_binario = nico_bito(val_izq, val_der);
                else if (strcmp(op_name, "BITXOR") == 0) resultado_binario = nico_bitxor(val_izq, val_der);
                else if (strcmp(op_name, "DESPLAZARIZQUIERDA") == 0) resultado_binario = nico_desplazarizquierda(val_izq, val_der);
                else if (strcmp(op_name, "DESPLAZARDERECHA") == 0) resultado_binario = nico_desplazarderecha(val_izq, val_der);
                else if (strcmp(op_name, "ROTARIZQUIERDA") == 0) resultado_binario = nico_rotarizquierda(val_izq, val_der);
                else if (strcmp(op_name, "ROTARDERECHA") == 0) resultado_binario = nico_rotarderecha(val_izq, val_der);
                else if (strcmp(op_name, "LEERBIT") == 0) resultado_binario = nico_leerbit(val_izq, val_der);
                else if (strcmp(op_name, "ACTIVARBIT") == 0) resultado_binario = nico_activarbit(val_izq, val_der);
                else if (strcmp(op_name, "DESACTIVARBIT") == 0) resultado_binario = nico_desactivarbit(val_izq, val_der);
                
                *exito = 1;
                profundidad_eval--;
                return resultado_binario;
            } else {
                fprintf(stderr, "Error: No se pudo evaluar operando en %s.\n", op_name);
                profundidad_eval--;
                return 0;
            }
        }
    }
    profundidad_eval--;  
    *exito = 1;
    return resultado;
}

// IMPLEMENTACIONES DE FUNCIONES MATEMÁTICAS
double nico_seno(double x) { return sin(x); }
double nico_coseno(double x) { return cos(x); }
double nico_tangente(double x) { return tan(x); }
double nico_arcoseno(double x) { return asin(x); }
double nico_arcocoseno(double x) { return acos(x); }
double nico_arcotangente(double x) { return atan(x); }
double nico_lognatural(double x) { return log(x); }
double nico_logbase10(double x) { return log10(x); }
double nico_logbase2(double x) { return log2(x); }
double nico_logaritmo_base(double numero, double base) { return log(numero) / log(base); }
double nico_numeropi(void) { return 3.14159265358979; }
double nico_numeroeuler(void) { return 2.71828182845905; }
double nico_raizdeunmedio(void) { return 0.70710678118655; }
double nico_lognaturalde2(void) { return 0.69314718055995; }
double nico_lognaturalde10(void) { return 2.30258509299405; }
double nico_exponencial(double x) { return exp(x); }
double nico_dosalax(double x) { return exp2(x); }
double nico_raizcubica(double x) { return cbrt(x); }
double nico_raiz(double x) { return sqrt(x); }
double nico_potencia(double base, double exp) { return pow(base, exp); }
double nico_modulo(double a, double b) { return fmod(a, b); }
double nico_absoluto(double x) { return fabs(x); }
double nico_redondear_arriba(double x) { return ceil(x); }
double nico_redondear_abajo(double x) { return floor(x); }
double nico_redondear_entero(double x) { return round(x); }
double nico_quitar_decimal(double x) { return trunc(x); }
double nico_maximo(double a, double b) { return (a > b) ? a : b; }
double nico_minimo(double a, double b) { return (a < b) ? a : b; }
double nico_sigmoide(double x) { return 1.0 / (1.0 + exp(-x)); }
