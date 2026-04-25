/*
 * Nico v1.0 - Intérprete Educativo de Scripting en Español
 * @file:         declaraciones.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Parsing y registro de variables, constantes, listas y 
 *                matrices globales.
 */
#include "nico.h"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"

/* Secuencias de escape para caracteres */
static char resolver_escape_char(const char *str) {
    if (!str) return '\0';
    if (str[0] == '\\' && str[1]) {
        switch(str[1]) {
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
            case '0': return '\0';
            case '\\': return '\\';
            case '\'': return '\'';
            case '"': return '"';
            default: return str[1];
        }
    }
    return str[0];
}

/* VERIFICAR COLISIÓN DE NOMBRES - FUNCIONES AUXILIARES */
int variable_ya_existe(const char *nombre) {
    if (en_funcion) return 0;
    
    if (buscar_variable(nombre) >= 0) return 1;
    if (buscar_variable_sin_signo(nombre) >= 0) return 1;
    if (buscar_variable_decimal(nombre) >= 0) return 1;
    if (buscar_variable_decimal_sin_signo(nombre) >= 0) return 1;
    if (buscar_variable_caracter(nombre) >= 0) return 1;
    if (buscar_variable_caracter_sin_signo(nombre) >= 0) return 1;
    if (buscar_texto_var(nombre) >= 0) return 1;
    
    if (buscar_constante(nombre) >= 0) return 1;
    if (buscar_constante_sin_signo(nombre) >= 0) return 1;
    if (buscar_constante_decimal(nombre) >= 0) return 1;
    if (buscar_constante_decimal_sin_signo(nombre) >= 0) return 1;
    if (buscar_constante_caracter(nombre) >= 0) return 1;
    if (buscar_constante_caracter_sin_signo(nombre) >= 0) return 1;
    if (buscar_texto_constante(nombre) >= 0) return 1;

    if (buscar_lista_entera(nombre) >= 0) return 1;
    if (buscar_lista_decimal(nombre) >= 0) return 1;
    if (buscar_lista_entera_sin_signo(nombre) >= 0) return 1;
    if (buscar_lista_decimal_sin_signo(nombre) >= 0) return 1;
    if (buscar_lista_caracter(nombre) >= 0) return 1;
    if (buscar_lista_caracter_sin_signo(nombre) >= 0) return 1;

    if (buscar_matriz_entera(nombre) >= 0) return 1;
    if (buscar_matriz_decimal(nombre) >= 0) return 1;
    if (buscar_matriz_entera_sin_signo(nombre) >= 0) return 1;
    if (buscar_matriz_decimal_sin_signo(nombre) >= 0) return 1;
    if (buscar_matriz_caracter(nombre) >= 0) return 1;
    if (buscar_matriz_caracter_sin_signo(nombre) >= 0) return 1;
    
    return 0;
}

const char* obtener_tipo_variable_existente(const char *nombre) {
    if (buscar_variable(nombre) >= 0) return "ENTERA";
    if (buscar_variable_sin_signo(nombre) >= 0) return "ENTERA SIN SIGNO";
    if (buscar_variable_decimal(nombre) >= 0) return "DECIMAL";
    if (buscar_variable_decimal_sin_signo(nombre) >= 0) return "DECIMAL SIN SIGNO";
    if (buscar_variable_caracter(nombre) >= 0) return "CARACTER";
    if (buscar_variable_caracter_sin_signo(nombre) >= 0) return "CARACTER SIN SIGNO";
    if (buscar_texto_var(nombre) >= 0) return "TEXTO";
    if (buscar_constante(nombre) >= 0) return "CONSTANTE ENTERA";
    if (buscar_constante_decimal(nombre) >= 0) return "CONSTANTE DECIMAL";
    if (buscar_texto_constante(nombre) >= 0) return "CONSTANTE TEXTO";
    return "DESCONOCIDO";
}

/* PARSEAR DECLARACION - Para VARIABLES y CONSTANTES con = valor */
int parsear_declaracion(const char *ptr, char *nombre, char *valor_texto, double *valor_double, int *hay_asignacion, int *es_texto) {
    *hay_asignacion = 0;
    *es_texto = 0;
    *valor_double = 0;
    valor_texto[0] = '\0';
    nombre[0] = '\0';
    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr == '$') ptr++;
    
    int i = 0;
    while (*ptr && *ptr != '=' && !isspace((unsigned char)*ptr) && i < MAX_NOMBRE - 1) {
        nombre[i++] = *ptr++;
    }
    nombre[i] = '\0';
    
    if (strlen(nombre) == 0) return 0;
    
    while (*ptr && *ptr != '=') ptr++;
    if (*ptr != '=') return 0;
    
    *hay_asignacion = 1;
    ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr == '"') {
        *es_texto = 1;
        ptr++;
        i = 0;
        while (*ptr && *ptr != '"' && i < MAX_TEXTO_LEN - 1) {
            valor_texto[i++] = *ptr++;
        }
        valor_texto[i] = '\0';
    } else if (*ptr == '\'') {
        ptr++;
        *valor_double = (double)(unsigned char)*ptr;
    } else {
        char num_str[MAX_LINEA];
        i = 0;
        while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && i < MAX_LINEA - 1) {
            num_str[i++] = *ptr++;
        }
        num_str[i] = '\0';
        *valor_double = atof(num_str);
    }
    
    return 1;
}

/* FUNCIONES AUXILIARES PARA INICIALIZACIÓN CON VALORES */
int parsear_valores_lista(const char *ptr, double *valores, int max_valores, int *num_valores) {
    *num_valores = 0;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != '{') return -1;
    ptr++;
    
    while (*ptr && *ptr != '}' && *num_valores < max_valores) {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '}') break;
        
        char valor_str[MAX_LINEA];
        int i = 0;
        while (*ptr && *ptr != ',' && *ptr != '}' && i < MAX_LINEA - 1) {
            valor_str[i++] = *ptr++;
        }
        valor_str[i] = '\0';
        limpiar_string(valor_str);
        
        if (strlen(valor_str) > 0) {
            valores[*num_valores] = atof(valor_str);
            (*num_valores)++;
        }
        if (*ptr == ',') ptr++;
    }
    
    return (*ptr == '}') ? 0 : -1;
}

int parsear_valores_matriz(const char *ptr, double valores[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA],
                           int filas, int columnas, int *filas_leidas, int *columnas_leidas) {
    *filas_leidas = 0;
    *columnas_leidas = 0;
    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != '{') return -1;
    ptr++;
    
    while (*ptr && *ptr != '}' && *filas_leidas < filas) {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '{') {
            ptr++;
            int col = 0;
            while (*ptr && *ptr != '}' && col < columnas) {
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                if (*ptr == '}') break;
                
                char valor_str[MAX_LINEA];
                int i = 0;
                while (*ptr && *ptr != ',' && *ptr != '}' && i < MAX_LINEA - 1) {
                    valor_str[i++] = *ptr++;
                }
                valor_str[i] = '\0';
                limpiar_string(valor_str);
                
                if (strlen(valor_str) > 0) {
                    valores[*filas_leidas][col] = atof(valor_str);
                    col++;
                }
                if (*ptr == ',') ptr++;
            }
            *columnas_leidas = col;
            if (*ptr == '}') ptr++;
            (*filas_leidas)++;
        }
        if (*ptr == ',') ptr++;
    }
    
    return (*ptr == '}') ? 0 : -1;
}

/* Parsear valores de matriz CARACTER con soporte para 'X' y escapes */
int parsear_valores_matriz_caracter(const char *ptr, char valores[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA],
                                    int filas, int columnas, int *filas_leidas, int *columnas_leidas, int sin_signo) {
    *filas_leidas = 0; *columnas_leidas = 0;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != '{') return -1; ptr++;
    
    while (*ptr && *ptr != '}' && *filas_leidas < filas) {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '{') {
            ptr++; int col = 0;
            while (*ptr && *ptr != '}' && col < columnas) {
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                if (*ptr == '}') break;
                
                char valor_str[MAX_LINEA]; int i = 0; int dentro_comilla = 0;
                while (*ptr && i < MAX_LINEA - 1) {
                    if (!dentro_comilla && (*ptr == ',' || *ptr == '}')) break;
                    if (*ptr == '\'' && (i == 0 || valor_str[i-1] != '\\')) dentro_comilla = !dentro_comilla;
                    valor_str[i++] = *ptr++;
                }
                valor_str[i] = '\0'; limpiar_string(valor_str);
                
                if (strlen(valor_str) > 0) {
                    char c = '\0';
                    if (valor_str[0] == '\'' && strlen(valor_str) >= 3) {
                        char contenido[4] = ""; int k = 0;
                        for (int j = 1; j < (int)strlen(valor_str) - 1 && k < 3; j++) contenido[k++] = valor_str[j];
                        c = resolver_escape_char(contenido);
                    } else if (es_numero(valor_str) || (valor_str[0]=='-' && es_numero(valor_str+1))) {
                        c = (char)atoi(valor_str);
                    } else { c = valor_str[0]; }
                    
                    valores[*filas_leidas][col] = sin_signo ? (unsigned char)c : c; col++;
                }
                if (*ptr == ',') ptr++;
            }
            *columnas_leidas = col; if (*ptr == '}') ptr++; (*filas_leidas)++;
        }
        if (*ptr == ',') ptr++;
    }
    return (*ptr == '}') ? 0 : -1;
}

/* VARIABLE ARCHIVO */
int procesar_declaracion_variable_archivo(const char *linea, int linea_actual) {
    fase_constantes = 0;
    fase_variables = 1;
    
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "VARIABLE ARCHIVO")) { ptr += 16; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    
    char nombre_var[MAX_NOMBRE] = "";
    if (*ptr == '$') ptr++;
    int j = 0;
    while (es_alnum(*ptr) && j < MAX_NOMBRE - 1) nombre_var[j++] = *ptr++;
    nombre_var[j] = '\0';
    
    if (variable_ya_existe(nombre_var)) {
        fprintf(stderr, "Error línea %d: La variable '%s' ya fue declarada.\n", linea_actual, nombre_var);
        return -1;
    }
    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr == '=') {
        ptr++;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        
        if (!comienza_con(ptr, "USARARCHIVO")) {
            fprintf(stderr, "Error línea %d: Se espera USARARCHIVO(...).\n", linea_actual);
            return -1;
        }
        ptr += 11;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr != '(') {
            fprintf(stderr, "Error línea %d: USARARCHIVO requiere paréntesis.\n", linea_actual);
            return -1;
        }
        ptr++;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr != '"') {
            fprintf(stderr, "Error línea %d: USARARCHIVO requiere nombre de archivo entre comillas.\n", linea_actual);
            return -1;
        }
        ptr++;
        
        char nombre_archivo[MAX_LINEA] = "";
        j = 0;
        while (*ptr && *ptr != '"' && j < MAX_LINEA - 1) {
            nombre_archivo[j++] = *ptr++;
        }
        nombre_archivo[j] = '\0';
        ptr++;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        
        char modo_str[20] = "";
        char *modo_ptr = strstr(ptr, "ESCRITURA");
        if (modo_ptr) strcpy(modo_str, "ESCRITURA");
        else {
            modo_ptr = strstr(ptr, "AGREGAR");
            if (modo_ptr) strcpy(modo_str, "AGREGAR");
            else {
                modo_ptr = strstr(ptr, "LECTURA");
                if (modo_ptr) strcpy(modo_str, "LECTURA");
                else {
                    modo_ptr = strstr(ptr, "LECTOESCRITURA");
                    if (modo_ptr) strcpy(modo_str, "LECTOESCRITURA");
                }
            }
        }
        
        if (strlen(modo_str) == 0) {
            fprintf(stderr, "Error línea %d: Modo debe ser ESCRITURA, AGREGAR, LECTURA o LECTOESCRITURA.\n", linea_actual);
            return -1;
        }
        
        if (procesar_usararchivo(nombre_archivo, modo_str, nombre_var) < 0) {
            fprintf(stderr, "Error línea %d: No se pudo abrir archivo.\n", linea_actual);
            return -1;
        }
    }
    
    return 0;
}

/* CONSTANTES */
int procesar_declaracion_constante_entera_sin_signo(const char *linea, int linea_actual) {
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "CONSTANTE ENTERA SIN SIGNO")) { ptr += 26; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0;
    int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error linea %d\n", linea_actual); return -1; }
    if (!hay_asignacion) { fprintf(stderr, "Error línea %d: Constante requiere asignación.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La constante '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    if (agregar_constante_sin_signo(nombre, (unsigned int)valor_double) < 0) { fprintf(stderr, "Error línea %d\n", linea_actual); return -1; }
    return 0;
}

int procesar_declaracion_constante_decimal_sin_signo(const char *linea, int linea_actual) {
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "CONSTANTE DECIMAL SIN SIGNO")) { ptr += 27; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0;
    int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    if (!hay_asignacion) { fprintf(stderr, "Error línea %d: Constante requiere asignación.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La constante '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    if (agregar_constante_decimal_sin_signo(nombre, valor_double) < 0) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    return 0;
}

int procesar_declaracion_constante_texto(const char *linea, int linea_actual) {
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "CONSTANTE TEXTO")) { ptr += 15; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0;
    int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    if (!hay_asignacion) { fprintf(stderr, "Error línea %d: Constante requiere asignación.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La constante '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    if (agregar_texto_constante(nombre, valor_texto) < 0) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    return 0;
}

int procesar_declaracion_constante_entera(const char *linea, int linea_actual) {
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "CONSTANTE ENTERA")) { ptr += 16; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0;
    int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    if (!hay_asignacion) { fprintf(stderr, "Error línea %d: Constante requiere asignación.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La constante '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    if (es_texto) { if (agregar_texto_constante(nombre, valor_texto) < 0) { fprintf(stderr, "Error línea %d\n", linea_actual); return -1; } }
    else { if (agregar_constante(nombre, (int)valor_double) < 0) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; } }
    return 0;
}

int procesar_declaracion_constante_decimal(const char *linea, int linea_actual) {
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "CONSTANTE DECIMAL")) { ptr += 17; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0;
    int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    if (!hay_asignacion) { fprintf(stderr, "Error línea %d: Constante requiere asignación.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La constante '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    if (agregar_constante_decimal(nombre, valor_double) < 0) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    return 0;
}

/* VARIABLES */
int procesar_declaracion_variable_entera_sin_signo(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "VARIABLE ENTERA SIN SIGNO")) { ptr += 25; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0; int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    if (scope_actual >= 0) agregar_variable_local(nombre, 1, valor_double); else agregar_variable_sin_signo(nombre, (unsigned int)valor_double);
    return 0;
}

int procesar_declaracion_variable_decimal_sin_signo(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "VARIABLE DECIMAL SIN SIGNO")) { ptr += 26; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0; int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    if (scope_actual >= 0) agregar_variable_local(nombre, 3, valor_double); else agregar_variable_decimal_sin_signo(nombre, valor_double);
    return 0;
}

int procesar_declaracion_variable_caracter_sin_signo(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "VARIABLE CARACTER SIN SIGNO")) { ptr += 27; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0; int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    unsigned char valor_char = (es_texto && strlen(valor_texto) > 0) ? (unsigned char)valor_texto[0] : (unsigned char)((int)valor_double);
    if (scope_actual >= 0) agregar_variable_local(nombre, 5, (double)valor_char); else agregar_variable_caracter_sin_signo(nombre, valor_char);
    return 0;
}

int procesar_declaracion_variable_entera(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "VARIABLE ENTERA")) { ptr += 15; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0; int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    if (scope_actual >= 0) agregar_variable_local(nombre, 0, valor_double); else agregar_variable(nombre, (int)valor_double);
    return 0;
}

int procesar_declaracion_variable_decimal(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "VARIABLE DECIMAL")) { ptr += 16; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0; int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; }
    if (scope_actual >= 0) agregar_variable_local(nombre, 2, valor_double); else agregar_variable_decimal(nombre, valor_double);
    return 0;
}

#define MAX_POOL_TEXTOS_LOCALES 200 
static char pool_textos_locales[MAX_POOL_TEXTOS_LOCALES][MAX_TEXTO_LEN];
static int pool_textos_contador = 0;

int agregar_texto_local(const char *nombre, const char *valor) {
    if (scope_actual < 0) return agregar_texto_var(nombre, valor);
    if (scope_actual >= MAX_SCOPES) return -1;
    ScopeLocal *scp = &scopes_locales[scope_actual];
    if (scp->num_textos >= MAX_TEXTOS_LOCALES) return -1;
    if (pool_textos_contador >= MAX_POOL_TEXTOS_LOCALES) return -1;
    int indice_pool = pool_textos_contador;
    strncpy(pool_textos_locales[indice_pool], valor ? valor : "", MAX_TEXTO_LEN - 1);
    pool_textos_locales[indice_pool][MAX_TEXTO_LEN - 1] = '\0';
    pool_textos_contador++;
    const char *clean_name = (nombre && nombre[0] == '$') ? nombre + 1 : nombre;
    strncpy(scp->nombres_textos[scp->num_textos], clean_name, MAX_NOMBRE - 1);
    scp->nombres_textos[scp->num_textos][MAX_NOMBRE - 1] = '\0';
    scp->indices_textos[scp->num_textos] = indice_pool;
    scp->num_textos++;
    return 0;
}

int buscar_texto_local(const char *nombre, char *buffer_out) {
    for (int s = scope_actual; s >= 0; s--) {
        ScopeLocal *scp = &scopes_locales[s];
        for (int i = 0; i < scp->num_textos; i++) {
            if (strcmp(scp->nombres_textos[i], nombre) == 0) {
                if (buffer_out) {
                    int idx = scp->indices_textos[i];
                    strncpy(buffer_out, pool_textos_locales[idx], MAX_TEXTO_LEN - 1);
                    buffer_out[MAX_TEXTO_LEN - 1] = '\0';
                }
                return 1;
            }
        }
    }
    return 0;
}

int procesar_declaracion_variable_texto(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "VARIABLE TEXTO")) { ptr += 14; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; char valor_texto[MAX_TEXTO_LEN]; double valor_double = 0; int hay_asignacion = 0, es_texto = 0;
    if (!parsear_declaracion(ptr, nombre, valor_texto, &valor_double, &hay_asignacion, &es_texto)) { fprintf(stderr, "Error linea %d\n", linea_actual); return -1; }
    if (agregar_texto_local(nombre, valor_texto) != 0) { fprintf(stderr, "Error al declarar variable texto '$%s'.\n", nombre); return -1; }
    return 0;
}

int procesar_declaracion_variable_caracter(const char *linea, int linea_actual) {
    char nombre[MAX_NOMBRE]; char valor_inicial = '\0';
    const char *ptr = strstr(linea, "VARIABLE CARACTER");
    if (!ptr) return -1; ptr += 17;
    while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr == '$') ptr++;
    int i = 0; while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre[i++] = *ptr++; nombre[i] = '\0';
    if (strlen(nombre) == 0) { fprintf(stderr, "Error línea %d: VARIABLE CARACTER requiere nombre.\n", linea_actual); return -1; }
    char *igual = strchr(linea, '=');
    if (igual) { igual++; while (*igual == ' ' || *igual == '\t') igual++; if (*igual == '\'') valor_inicial = *(igual + 1); }
    if (scope_actual >= 0) agregar_variable_local(nombre, 4, (double)valor_inicial);
    else {
        int idx = num_variables_caracter;
        if (idx < MAX_VARS_CARACTER) {
            strncpy(variables_caracter[idx].nombre, nombre, MAX_NOMBRE - 1); variables_caracter[idx].nombre[MAX_NOMBRE - 1] = '\0';
            variables_caracter[idx].valor = valor_inicial; num_variables_caracter++;
        } else fprintf(stderr, "Error línea %d: Máximo de variables CARACTER alcanzado.\n", linea_actual);
    }
    return 0;
}

/* LISTAS */
int procesar_declaracion_lista_entera_sin_signo(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "LISTA ENTERA SIN SIGNO")) { ptr += 22; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; int j = 0;
    if (*ptr != '$') { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; } ptr++;
    while (*ptr && *ptr != '[' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1) nombre[j++] = *ptr++; nombre[j] = '\0';
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La lista '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; } ptr++;
    char longitud_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) longitud_str[j++] = *ptr++; longitud_str[j] = '\0';
    int longitud = atoi(longitud_str);
    if (longitud <= 0 || longitud > MAX_LISTA) { fprintf(stderr, "Error línea %d: Longitud inválida.\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_listas_enteras_sin_signo;
        if (idx_actual >= MAX_LISTAS_ENTERAS_SIN_SIGNO) { fprintf(stderr, "Error línea %d: Límite de listas enteras sin signo alcanzado.\n", linea_actual); return -1; }
        strncpy(listas_enteras_sin_signo[idx_actual].nombre, nombre, MAX_NOMBRE - 1); listas_enteras_sin_signo[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        listas_enteras_sin_signo[idx_actual].longitud = longitud; listas_enteras_sin_signo[idx_actual].esta_declarada = 1;
        for (int k = 0; k < longitud; k++) listas_enteras_sin_signo[idx_actual].valores[k] = 0;
        num_listas_enteras_sin_signo++;
        if (registrar_lista_local(nombre, 2, idx_actual, longitud) < 0) { fprintf(stderr, "Error línea %d: Máximo de listas locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_lista_entera_sin_signo(nombre, longitud);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo crear lista.\n", linea_actual); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        double valores_iniciales[MAX_LISTA]; int num_valores = 0;
        if (parsear_valores_lista(ptr, valores_iniciales, MAX_LISTA, &num_valores) == 0) {
            int limite = (num_valores < longitud) ? num_valores : longitud;
            for (int i = 0; i < limite; i++) listas_enteras_sin_signo[idx_actual].valores[i] = (unsigned int)valores_iniciales[i];
        }
    }
    return 0;
}

int procesar_declaracion_lista_decimal_sin_signo(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "LISTA DECIMAL SIN SIGNO")) { ptr += 23; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; int j = 0;
    if (*ptr != '$') { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; } ptr++;
    while (*ptr && *ptr != '[' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1) nombre[j++] = *ptr++; nombre[j] = '\0';
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La lista '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; } ptr++;
    char longitud_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) longitud_str[j++] = *ptr++; longitud_str[j] = '\0';
    int longitud = atoi(longitud_str);
    if (longitud <= 0 || longitud > MAX_LISTA) { fprintf(stderr, "Error línea %d: Longitud inválida.\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_listas_decimales_sin_signo;
        if (idx_actual >= MAX_LISTAS_DECIMALES_SIN_SIGNO) { fprintf(stderr, "Error línea %d: Límite de listas decimales sin signo alcanzado.\n", linea_actual); return -1; }
        strncpy(listas_decimales_sin_signo[idx_actual].nombre, nombre, MAX_NOMBRE - 1); listas_decimales_sin_signo[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        listas_decimales_sin_signo[idx_actual].longitud = longitud; listas_decimales_sin_signo[idx_actual].esta_declarada = 1;
        for (int k = 0; k < longitud; k++) listas_decimales_sin_signo[idx_actual].valores[k] = 0.0;
        num_listas_decimales_sin_signo++;
        if (registrar_lista_local(nombre, 3, idx_actual, longitud) < 0) { fprintf(stderr, "Error línea %d: Máximo de listas locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_lista_decimal_sin_signo(nombre, longitud);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo crear lista.\n", linea_actual); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        double valores_iniciales[MAX_LISTA]; int num_valores = 0;
        if (parsear_valores_lista(ptr, valores_iniciales, MAX_LISTA, &num_valores) == 0) {
            int limite = (num_valores < longitud) ? num_valores : longitud;
            for (int i = 0; i < limite; i++) listas_decimales_sin_signo[idx_actual].valores[i] = valores_iniciales[i];
        }
    }
    return 0;
}

int procesar_declaracion_lista_entera(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "LISTA ENTERA")) { ptr += 12; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; int j = 0;
    if (*ptr != '$') { fprintf(stderr, "Error línea %d: Se esperaba '$', se encontró '%c'.\n", linea_actual, *ptr); return -1; } ptr++;
    while (*ptr && *ptr != '[' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1) nombre[j++] = *ptr++; nombre[j] = '\0';
    if (strlen(nombre) == 0) { fprintf(stderr, "Error línea %d: Nombre de lista vacío.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La lista '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: Se esperaba '[', se encontró '%c'\n", linea_actual, *ptr); return -1; } ptr++;
    char longitud_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) longitud_str[j++] = *ptr++; longitud_str[j] = '\0';
    int longitud = atoi(longitud_str);
    if (longitud <= 0 || longitud > MAX_LISTA) { fprintf(stderr, "Error línea %d: Longitud inválida=%d.\n", linea_actual, longitud); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_listas_enteras;
        if (idx_actual >= MAX_LISTAS_ENTERAS) { fprintf(stderr, "Error línea %d: Límite de listas enteras alcanzado.\n", linea_actual); return -1; }
        strncpy(listas_enteras[idx_actual].nombre, nombre, MAX_NOMBRE - 1); listas_enteras[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        listas_enteras[idx_actual].longitud = longitud; listas_enteras[idx_actual].esta_declarada = 1;
        for (int k = 0; k < longitud; k++) listas_enteras[idx_actual].valores[k] = 0;
        num_listas_enteras++;
        if (registrar_lista_local(nombre, 0, idx_actual, longitud) < 0) { fprintf(stderr, "Error línea %d: Máximo de listas locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_lista_entera(nombre, longitud);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo agregar lista.\n", linea_actual); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        double valores_iniciales[MAX_LISTA]; int num_valores = 0;
        if (parsear_valores_lista(ptr, valores_iniciales, MAX_LISTA, &num_valores) == 0) {
            int limite = (num_valores < longitud) ? num_valores : longitud;
            for (int i = 0; i < limite; i++) listas_enteras[idx_actual].valores[i] = (int)valores_iniciales[i];
        }
    }
    return 0;
}

int procesar_declaracion_lista_decimal(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "LISTA DECIMAL")) { ptr += 13; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; int j = 0;
    if (*ptr != '$') { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; } ptr++;
    while (*ptr && *ptr != '[' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1) nombre[j++] = *ptr++; nombre[j] = '\0';
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La lista '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d.\n", linea_actual); return -1; } ptr++;
    char longitud_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) longitud_str[j++] = *ptr++; longitud_str[j] = '\0';
    int longitud = atoi(longitud_str);
    if (longitud <= 0 || longitud > MAX_LISTA) { fprintf(stderr, "Error línea %d: Longitud inválida.\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_listas_decimales;
        if (idx_actual >= MAX_LISTAS_DECIMALES) { fprintf(stderr, "Error línea %d: Límite de listas decimales alcanzado.\n", linea_actual); return -1; }
        strncpy(listas_decimales[idx_actual].nombre, nombre, MAX_NOMBRE - 1); listas_decimales[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        listas_decimales[idx_actual].longitud = longitud; listas_decimales[idx_actual].esta_declarada = 1;
        for (int k = 0; k < longitud; k++) listas_decimales[idx_actual].valores[k] = 0.0;
        num_listas_decimales++;
        if (registrar_lista_local(nombre, 1, idx_actual, longitud) < 0) { fprintf(stderr, "Error línea %d: Máximo de listas locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_lista_decimal(nombre, longitud);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo crear lista.\n", linea_actual); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        double valores_iniciales[MAX_LISTA]; int num_valores = 0;
        if (parsear_valores_lista(ptr, valores_iniciales, MAX_LISTA, &num_valores) == 0) {
            int limite = (num_valores < longitud) ? num_valores : longitud;
            for (int i = 0; i < limite; i++) listas_decimales[idx_actual].valores[i] = valores_iniciales[i];
        }
    }
    return 0;
}

int procesar_declaracion_lista_caracter(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (comienza_con(ptr, "LISTA CARACTER") || comienza_con(ptr, "DECLARAR LISTA CARACTER")) {
        if (comienza_con(ptr, "DECLARAR")) ptr += 8; else ptr += 14;
    } else return -1;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != '$') { fprintf(stderr, "Error línea %d: Lista caracter debe comenzar con $.\n", linea_actual); return -1; } ptr++;
    char nombre[MAX_NOMBRE] = ""; int i = 0; while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre[i++] = *ptr++; nombre[i] = '\0';
    if (strlen(nombre) == 0) { fprintf(stderr, "Error línea %d: Nombre de lista no encontrado.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La lista '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: Lista caracter requiere [longitud]\n", linea_actual); return -1; } ptr++;
    char longitud_str[20] = ""; i = 0; while (*ptr && *ptr != ']' && i < 19) longitud_str[i++] = *ptr++; longitud_str[i] = '\0';
    int longitud = atoi(longitud_str);
    if (longitud <= 0 || longitud > MAX_LISTA) { fprintf(stderr, "Error línea %d: Longitud de lista caracter inválida\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_listas_caracter;
        if (idx_actual >= MAX_LISTAS_CARACTER) { fprintf(stderr, "Error línea %d: Límite de listas caracter alcanzado.\n", linea_actual); return -1; }
        strncpy(listas_caracter[idx_actual].nombre, nombre, MAX_NOMBRE - 1); listas_caracter[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        listas_caracter[idx_actual].longitud = longitud; listas_caracter[idx_actual].esta_declarada = 1;
        for (int k = 0; k < longitud; k++) listas_caracter[idx_actual].valores[k] = '\0';
        num_listas_caracter++;
        if (registrar_lista_local(nombre, 4, idx_actual, longitud) < 0) { fprintf(stderr, "Error línea %d: Máximo de listas locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_lista_caracter(nombre, longitud);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo declarar lista caracter '$%s'.\n", linea_actual, nombre); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '{') {
            ptr++; int valor_idx = 0;
            while (*ptr && *ptr != '}' && valor_idx < longitud) {
                while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr == '}') break;
                char caracter = 0;
                if (*ptr == '\'') { ptr++; caracter = *ptr; ptr++; if (*ptr == '\'') ptr++; } else { caracter = *ptr; ptr++; }
                listas_caracter[idx_actual].valores[valor_idx] = caracter; valor_idx++;
                while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr == ',') ptr++;
            }
        }
    }
    return 0;
}

int procesar_declaracion_lista_caracter_sin_signo(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (comienza_con(ptr, "LISTA CARACTER SIN SIGNO") || comienza_con(ptr, "DECLARAR LISTA CARACTER SIN SIGNO")) {
        if (comienza_con(ptr, "DECLARAR")) ptr += 8; else ptr += 24;
    } else return -1;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != '$') { fprintf(stderr, "Error línea %d: Lista caracter sin signo debe comenzar con $.\n", linea_actual); return -1; } ptr++;
    char nombre[MAX_NOMBRE] = ""; int i = 0; while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre[i++] = *ptr++; nombre[i] = '\0';
    if (strlen(nombre) == 0) { fprintf(stderr, "Error línea %d: Nombre de lista no encontrado.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La lista '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: Lista caracter sin signo requiere [longitud]\n", linea_actual); return -1; } ptr++;
    char longitud_str[20] = ""; i = 0; while (*ptr && *ptr != ']' && i < 19) longitud_str[i++] = *ptr++; longitud_str[i] = '\0';
    int longitud = atoi(longitud_str);
    if (longitud <= 0 || longitud > MAX_LISTA) { fprintf(stderr, "Error línea %d: Longitud de lista caracter sin signo inválida.\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_listas_caracter_sin_signo;
        if (idx_actual >= MAX_LISTAS_CARACTER_SIN_SIGNO) { fprintf(stderr, "Error línea %d: Límite de listas caracter sin signo alcanzado.\n", linea_actual); return -1; }
        strncpy(listas_caracter_sin_signo[idx_actual].nombre, nombre, MAX_NOMBRE - 1); listas_caracter_sin_signo[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        listas_caracter_sin_signo[idx_actual].longitud = longitud; listas_caracter_sin_signo[idx_actual].esta_declarada = 1;
        for (int k = 0; k < longitud; k++) listas_caracter_sin_signo[idx_actual].valores[k] = '\0';
        num_listas_caracter_sin_signo++;
        if (registrar_lista_local(nombre, 5, idx_actual, longitud) < 0) { fprintf(stderr, "Error línea %d: Máximo de listas locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_lista_caracter_sin_signo(nombre, longitud);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo declarar lista caracter sin signo '$%s'.\n", linea_actual, nombre); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '{') {
            ptr++; int valor_idx = 0;
            while (*ptr && *ptr != '}' && valor_idx < longitud) {
                while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr == '}') break;
                unsigned char caracter = 0;
                if (*ptr == '\'') { ptr++; caracter = (unsigned char)*ptr; ptr++; if (*ptr == '\'') ptr++; } else { caracter = (unsigned char)*ptr; ptr++; }
                listas_caracter_sin_signo[idx_actual].valores[valor_idx] = caracter; valor_idx++;
                while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr == ',') ptr++;
            }
        }
    }
    return 0;
}

/* MATRICES */
int procesar_declaracion_matriz_entera(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "MATRIZ ENTERA")) { ptr += 13; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; int j = 0; if (*ptr == '$') ptr++;
    while (*ptr && *ptr != '[' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1) nombre[j++] = *ptr++; nombre[j] = '\0';
    if (strlen(nombre) == 0) { fprintf(stderr, "Error línea %d: MATRIZ ENTERA requiere nombre.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La matriz '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: MATRIZ ENTERA requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char filas_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) filas_str[j++] = *ptr++; filas_str[j] = '\0'; int filas = atoi(filas_str); ptr++;
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: MATRIZ ENTERA requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char columnas_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) columnas_str[j++] = *ptr++; columnas_str[j] = '\0'; int columnas = atoi(columnas_str);
    if (filas <= 0 || filas > MAX_DIMENSION_FILA || columnas <= 0 || columnas > MAX_DIMENSION_COLUMNA) { fprintf(stderr, "Error línea %d: Dimensiones de matriz inválidas.\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_matriz_enteras;
        if (idx_actual >= MAX_MATRICES_ENTERAS) { fprintf(stderr, "Error línea %d: Límite de matrices enteras alcanzado.\n", linea_actual); return -1; }
        strncpy(matrices_enteras[idx_actual].nombre, nombre, MAX_NOMBRE - 1); matrices_enteras[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        matrices_enteras[idx_actual].filas = filas; matrices_enteras[idx_actual].columnas = columnas; matrices_enteras[idx_actual].esta_declarada = 1;
        for(int f=0;f<filas;f++) for(int c=0;c<columnas;c++) matrices_enteras[idx_actual].valores[f][c]=0;
        num_matriz_enteras++;
        if (registrar_matriz_local(nombre, 0, idx_actual, filas, columnas) < 0) { fprintf(stderr, "Error línea %d: Máximo de matrices locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_matriz_entera(nombre, filas, columnas);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo declarar matriz.\n", linea_actual); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        double valores_iniciales[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA]; int filas_leidas = 0, columnas_leidas = 0;
        if (parsear_valores_matriz(ptr, valores_iniciales, filas, columnas, &filas_leidas, &columnas_leidas) == 0) {
            int limite_f = (filas_leidas < filas) ? filas_leidas : filas;
            int limite_c = (columnas_leidas < columnas) ? columnas_leidas : columnas;
            for (int f = 0; f < limite_f; f++) for (int c = 0; c < limite_c; c++) matrices_enteras[idx_actual].valores[f][c] = (int)valores_iniciales[f][c];
        }
    }
    return 0;
}

int procesar_declaracion_matriz_decimal(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "MATRIZ DECIMAL")) { ptr += 14; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; int j = 0; if (*ptr == '$') ptr++;
    while (*ptr && *ptr != '[' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1) nombre[j++] = *ptr++; nombre[j] = '\0';
    if (strlen(nombre) == 0) { fprintf(stderr, "Error línea %d: MATRIZ DECIMAL requiere nombre.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La matriz '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: MATRIZ DECIMAL requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char filas_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) filas_str[j++] = *ptr++; filas_str[j] = '\0'; int filas = atoi(filas_str); ptr++;
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: MATRIZ DECIMAL requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char columnas_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) columnas_str[j++] = *ptr++; columnas_str[j] = '\0'; int columnas = atoi(columnas_str);
    if (filas <= 0 || filas > MAX_DIMENSION_FILA || columnas <= 0 || columnas > MAX_DIMENSION_COLUMNA) { fprintf(stderr, "Error línea %d: Dimensiones de matriz inválidas.\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_matriz_decimales;
        if (idx_actual >= MAX_MATRICES_DECIMALES) { fprintf(stderr, "Error línea %d: Límite de matrices decimales alcanzado.\n", linea_actual); return -1; }
        strncpy(matrices_decimales[idx_actual].nombre, nombre, MAX_NOMBRE - 1); matrices_decimales[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        matrices_decimales[idx_actual].filas = filas; matrices_decimales[idx_actual].columnas = columnas; matrices_decimales[idx_actual].esta_declarada = 1;
        for(int f=0;f<filas;f++) for(int c=0;c<columnas;c++) matrices_decimales[idx_actual].valores[f][c]=0.0;
        num_matriz_decimales++;
        if (registrar_matriz_local(nombre, 1, idx_actual, filas, columnas) < 0) { fprintf(stderr, "Error línea %d: Máximo de matrices locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_matriz_decimal(nombre, filas, columnas);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo declarar matriz.\n", linea_actual); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        double valores_iniciales[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA]; int filas_leidas = 0, columnas_leidas = 0;
        if (parsear_valores_matriz(ptr, valores_iniciales, filas, columnas, &filas_leidas, &columnas_leidas) == 0) {
            int limite_f = (filas_leidas < filas) ? filas_leidas : filas;
            int limite_c = (columnas_leidas < columnas) ? columnas_leidas : columnas;
            for (int f = 0; f < limite_f; f++) for (int c = 0; c < limite_c; c++) matrices_decimales[idx_actual].valores[f][c] = valores_iniciales[f][c];
        }
    }
    return 0;
}

int procesar_declaracion_matriz_entera_sin_signo(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "MATRIZ ENTERA SIN SIGNO")) { ptr += 23; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; int j = 0; if (*ptr == '$') ptr++;
    while (*ptr && *ptr != '[' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1) nombre[j++] = *ptr++; nombre[j] = '\0';
    if (strlen(nombre) == 0) { fprintf(stderr, "Error línea %d: MATRIZ ENTERA SIN SIGNO requiere nombre.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La matriz '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: MATRIZ ENTERA SIN SIGNO requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char filas_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) filas_str[j++] = *ptr++; filas_str[j] = '\0'; int filas = atoi(filas_str); ptr++;
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: MATRIZ ENTERA SIN SIGNO requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char columnas_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) columnas_str[j++] = *ptr++; columnas_str[j] = '\0'; int columnas = atoi(columnas_str);
    if (filas <= 0 || filas > MAX_DIMENSION_FILA || columnas <= 0 || columnas > MAX_DIMENSION_COLUMNA) { fprintf(stderr, "Error línea %d: Dimensiones de matriz inválidas.\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_matriz_enteras_sin_signo;
        if (idx_actual >= MAX_MATRICES_ENTERAS_SIN_SIGNO) { fprintf(stderr, "Error línea %d: Límite de matrices enteras sin signo alcanzado.\n", linea_actual); return -1; }
        strncpy(matrices_enteras_sin_signo[idx_actual].nombre, nombre, MAX_NOMBRE - 1); matrices_enteras_sin_signo[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        matrices_enteras_sin_signo[idx_actual].filas = filas; matrices_enteras_sin_signo[idx_actual].columnas = columnas; matrices_enteras_sin_signo[idx_actual].esta_declarada = 1;
        for(int f=0;f<filas;f++) for(int c=0;c<columnas;c++) matrices_enteras_sin_signo[idx_actual].valores[f][c]=0;
        num_matriz_enteras_sin_signo++;
        if (registrar_matriz_local(nombre, 2, idx_actual, filas, columnas) < 0) { fprintf(stderr, "Error línea %d: Máximo de matrices locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_matriz_entera_sin_signo(nombre, filas, columnas);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo declarar matriz.\n", linea_actual); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        double valores_iniciales[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA]; int filas_leidas = 0, columnas_leidas = 0;
        if (parsear_valores_matriz(ptr, valores_iniciales, filas, columnas, &filas_leidas, &columnas_leidas) == 0) {
            int limite_f = (filas_leidas < filas) ? filas_leidas : filas;
            int limite_c = (columnas_leidas < columnas) ? columnas_leidas : columnas;
            for (int f = 0; f < limite_f; f++) for (int c = 0; c < limite_c; c++) matrices_enteras_sin_signo[idx_actual].valores[f][c] = (unsigned int)valores_iniciales[f][c];
        }
    }
    return 0;
}

int procesar_declaracion_matriz_decimal_sin_signo(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    if (comienza_con(ptr, "MATRIZ DECIMAL SIN SIGNO")) { ptr += 24; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    char nombre[MAX_NOMBRE]; int j = 0; if (*ptr == '$') ptr++;
    while (*ptr && *ptr != '[' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1) nombre[j++] = *ptr++; nombre[j] = '\0';
    if (strlen(nombre) == 0) { fprintf(stderr, "Error línea %d: MATRIZ DECIMAL SIN SIGNO requiere nombre.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La matriz '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "   Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: MATRIZ DECIMAL SIN SIGNO requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char filas_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) filas_str[j++] = *ptr++; filas_str[j] = '\0'; int filas = atoi(filas_str); ptr++;
    while (*ptr && *ptr != '[') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: MATRIZ DECIMAL SIN SIGNO requiere [filas][columnas]\n", linea_actual); return -1; } ptr++;
    char columnas_str[MAX_LINEA]; j = 0; while (*ptr && *ptr != ']' && j < MAX_LINEA - 1) columnas_str[j++] = *ptr++; columnas_str[j] = '\0'; int columnas = atoi(columnas_str);
    if (filas <= 0 || filas > MAX_DIMENSION_FILA || columnas <= 0 || columnas > MAX_DIMENSION_COLUMNA) { fprintf(stderr, "Error línea %d: Dimensiones de matriz inválidas\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_matriz_decimales_sin_signo;
        if (idx_actual >= MAX_MATRICES_DECIMALES_SIN_SIGNO) { fprintf(stderr, "Error línea %d: Límite de matrices decimales sin signo alcanzado\n", linea_actual); return -1; }
        strncpy(matrices_decimales_sin_signo[idx_actual].nombre, nombre, MAX_NOMBRE - 1); matrices_decimales_sin_signo[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        matrices_decimales_sin_signo[idx_actual].filas = filas; matrices_decimales_sin_signo[idx_actual].columnas = columnas; matrices_decimales_sin_signo[idx_actual].esta_declarada = 1;
        for(int f=0;f<filas;f++) for(int c=0;c<columnas;c++) matrices_decimales_sin_signo[idx_actual].valores[f][c]=0.0;
        num_matriz_decimales_sin_signo++;
        if (registrar_matriz_local(nombre, 3, idx_actual, filas, columnas) < 0) { fprintf(stderr, "Error línea %d: Máximo de matrices locales alcanzado\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_matriz_decimal_sin_signo(nombre, filas, columnas);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo declarar matriz.\n", linea_actual); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        double valores_iniciales[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA]; int filas_leidas = 0, columnas_leidas = 0;
        if (parsear_valores_matriz(ptr, valores_iniciales, filas, columnas, &filas_leidas, &columnas_leidas) == 0) {
            int limite_f = (filas_leidas < filas) ? filas_leidas : filas;
            int limite_c = (columnas_leidas < columnas) ? columnas_leidas : columnas;
            for (int f = 0; f < limite_f; f++) for (int c = 0; c < limite_c; c++) matrices_decimales_sin_signo[idx_actual].valores[f][c] = valores_iniciales[f][c];
        }
    }
    return 0;
}

int procesar_declaracion_matriz_caracter(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (comienza_con(ptr, "MATRIZ CARACTER") || comienza_con(ptr, "DECLARAR MATRIZ CARACTER")) {
        if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
        if (comienza_con(ptr, "MATRIZ CARACTER")) { ptr += 15; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    } else return -1;
    
    if (*ptr != '$') { fprintf(stderr, "Error línea %d: Matriz caracter debe comenzar con $.\n", linea_actual); return -1; } ptr++;
    char nombre[MAX_NOMBRE] = ""; int i = 0; while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre[i++] = *ptr++; nombre[i] = '\0';
    if (strlen(nombre) == 0) { fprintf(stderr, "Error línea %d: Nombre de matriz no encontrado.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La matriz '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    
    while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: Matriz caracter requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char filas_str[20] = ""; i = 0; while (*ptr && *ptr != ']' && i < 19) filas_str[i++] = *ptr++; filas_str[i] = '\0'; int filas = atoi(filas_str); ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: Matriz caracter requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char columnas_str[20] = ""; i = 0; while (*ptr && *ptr != ']' && i < 19) columnas_str[i++] = *ptr++; columnas_str[i] = '\0'; int columnas = atoi(columnas_str);
    if (filas <= 0 || filas > MAX_DIMENSION_FILA || columnas <= 0 || columnas > MAX_DIMENSION_COLUMNA) { fprintf(stderr, "Error linea %d: Dimensiones de matriz caracter inválidas.\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_matriz_caracter;
        if (idx_actual >= MAX_MATRICES_CARACTER) { fprintf(stderr, "Error línea %d: Límite de matrices caracter alcanzado.\n", linea_actual); return -1; }
        strncpy(matrices_caracter[idx_actual].nombre, nombre, MAX_NOMBRE - 1); matrices_caracter[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        matrices_caracter[idx_actual].filas = filas; matrices_caracter[idx_actual].columnas = columnas; matrices_caracter[idx_actual].esta_declarada = 1;
        for(int f=0;f<filas;f++) for(int c=0;c<columnas;c++) matrices_caracter[idx_actual].valores[f][c]='\0';
        num_matriz_caracter++;
        if (registrar_matriz_local(nombre, 4, idx_actual, filas, columnas) < 0) { fprintf(stderr, "Error línea %d: Máximo de matrices locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_matriz_caracter(nombre, filas, columnas);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo declarar matriz caracter '$%s'.\n", linea_actual, nombre); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        char valores_iniciales[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA]; 
        int filas_leidas = 0, columnas_leidas = 0;
        if (parsear_valores_matriz_caracter(ptr, valores_iniciales, filas, columnas, &filas_leidas, &columnas_leidas, 0) == 0) {
            int limite_f = (filas_leidas < filas) ? filas_leidas : filas;
            int limite_c = (columnas_leidas < columnas) ? columnas_leidas : columnas;
            for (int f = 0; f < limite_f; f++) 
                for (int c = 0; c < limite_c; c++) 
                    matrices_caracter[idx_actual].valores[f][c] = valores_iniciales[f][c];
        }
    }
    return 0;
}

int procesar_declaracion_matriz_caracter_sin_signo(const char *linea, int linea_actual) {
    fase_constantes = 0; fase_variables = 1;
    const char *ptr = linea;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (comienza_con(ptr, "MATRIZ CARACTER SIN SIGNO") || comienza_con(ptr, "DECLARAR MATRIZ CARACTER SIN SIGNO")) {
        if (comienza_con(ptr, "DECLARAR")) { ptr += 8; while (*ptr == ' ' || *ptr == '\t') ptr++; }
        if (comienza_con(ptr, "MATRIZ CARACTER SIN SIGNO")) { ptr += 25; while (*ptr == ' ' || *ptr == '\t') ptr++; }
    } else return -1;
    
    if (*ptr != '$') { fprintf(stderr, "Error línea %d: Matriz caracter sin signo debe comenzar con $.\n", linea_actual); return -1; } ptr++;
    char nombre[MAX_NOMBRE] = ""; int i = 0; while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre[i++] = *ptr++; nombre[i] = '\0';
    if (strlen(nombre) == 0) { fprintf(stderr, "Error línea %d: Nombre de matriz no encontrado.\n", linea_actual); return -1; }
    if (variable_ya_existe(nombre)) { fprintf(stderr, "ADVERTENCIA línea %d: La matriz '$%s' YA EXISTE como %s.\n", linea_actual, nombre, obtener_tipo_variable_existente(nombre)); fprintf(stderr, "Sugerencia: Usá un nombre diferente para evitar colisiones\n"); }
    
    while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: Matriz caracter sin signo requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char filas_str[20] = ""; i = 0; while (*ptr && *ptr != ']' && i < 19) filas_str[i++] = *ptr++; filas_str[i] = '\0'; int filas = atoi(filas_str); ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++; if (*ptr != '[') { fprintf(stderr, "Error línea %d: Matriz caracter sin signo requiere [filas][columnas].\n", linea_actual); return -1; } ptr++;
    char columnas_str[20] = ""; i = 0; while (*ptr && *ptr != ']' && i < 19) columnas_str[i++] = *ptr++; columnas_str[i] = '\0'; int columnas = atoi(columnas_str);
    if (filas <= 0 || filas > MAX_DIMENSION_FILA || columnas <= 0 || columnas > MAX_DIMENSION_COLUMNA) { fprintf(stderr, "Error línea %d: Dimensiones de matriz caracter sin signo inválidas.\n", linea_actual); return -1; }

    int idx_actual = -1;
    if (scope_actual >= 0) {
        idx_actual = num_matriz_caracter_sin_signo;
        if (idx_actual >= MAX_MATRICES_CARACTER_SIN_SIGNO) { fprintf(stderr, "Error línea %d: Límite de matrices caracter sin signo alcanzado\n", linea_actual); return -1; }
        strncpy(matrices_caracter_sin_signo[idx_actual].nombre, nombre, MAX_NOMBRE - 1); matrices_caracter_sin_signo[idx_actual].nombre[MAX_NOMBRE - 1] = '\0';
        matrices_caracter_sin_signo[idx_actual].filas = filas; matrices_caracter_sin_signo[idx_actual].columnas = columnas; matrices_caracter_sin_signo[idx_actual].esta_declarada = 1;
        for(int f=0;f<filas;f++) for(int c=0;c<columnas;c++) matrices_caracter_sin_signo[idx_actual].valores[f][c]='\0';
        num_matriz_caracter_sin_signo++;
        if (registrar_matriz_local(nombre, 5, idx_actual, filas, columnas) < 0) { fprintf(stderr, "Error linea %d: Máximo de matrices locales alcanzado.\n", linea_actual); return -1; }
    } else {
        idx_actual = agregar_matriz_caracter_sin_signo(nombre, filas, columnas);
        if (idx_actual < 0) { fprintf(stderr, "Error línea %d: No se pudo declarar matriz caracter sin signo '$%s'.\n", linea_actual, nombre); return -1; }
    }

    while (*ptr && *ptr != '=') ptr++;
    if (*ptr == '=') {
        ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;
        char valores_iniciales[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA]; 
        int filas_leidas = 0, columnas_leidas = 0;
        if (parsear_valores_matriz_caracter(ptr, valores_iniciales, filas, columnas, &filas_leidas, &columnas_leidas, 1) == 0) {
            int limite_f = (filas_leidas < filas) ? filas_leidas : filas;
            int limite_c = (columnas_leidas < columnas) ? columnas_leidas : columnas;
            for (int f = 0; f < limite_f; f++) 
                for (int c = 0; c < limite_c; c++) 
                    matrices_caracter_sin_signo[idx_actual].valores[f][c] = valores_iniciales[f][c];
        }
    }
    return 0;
}
