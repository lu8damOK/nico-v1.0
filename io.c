/*
 * Nico v1.0 - Intérprete Educativo de Scripting en Español
 * @file:         io.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Entrada/salida estándar, manejo de archivos, 
 *                lectura de líneas y formateo de texto.
 */
#define _DEFAULT_SOURCE
#include "nico.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

/* FUNCIONES DE ARCHIVOS */
int procesar_usararchivo(const char *nombre_archivo, const char *modo_str, const char *nombre_variable) {
    char modo_c[5] = "";
    
    if (strcmp(modo_str, "ESCRITURA") == 0) strcpy(modo_c, "w");
    else if (strcmp(modo_str, "AGREGAR") == 0) strcpy(modo_c, "a");
    else if (strcmp(modo_str, "LECTURA") == 0) strcpy(modo_c, "r");
    else if (strcmp(modo_str, "LECTOESCRITURA") == 0) strcpy(modo_c, "r+");
    else {
        fprintf(stderr, "Error: Modo debe ser ESCRITURA, AGREGAR, LECTURA o LECTOESCRITURA.\n");
        return -1;
    }
    
    FILE *archivo = fopen(nombre_archivo, modo_c);
    if (archivo == NULL) {
        fprintf(stderr, "Error: No se pudo abrir '%s'.\n", nombre_archivo);
        return -1;
    }
    
    int idx = -1;
    if (nombre_variable && strlen(nombre_variable) > 0) {
        for (int k = 0; k < num_variables_archivo; k++) {
            if (strcmp(variables_archivo[k].nombre, nombre_variable) == 0) {
                if (variables_archivo[k].archivo != NULL) fclose(variables_archivo[k].archivo);
                idx = k;
                break;
            }
        }
    }
    
    if (idx < 0) {
        if (num_variables_archivo >= MAX_VARS_ARCHIVO) {
            fprintf(stderr, "Error: Máximo de archivos alcanzado.\n");
            fclose(archivo);
            return -1;
        }
        idx = num_variables_archivo;
        num_variables_archivo++;
    }
    
    if (nombre_variable && strlen(nombre_variable) > 0) {
        strncpy(variables_archivo[idx].nombre, nombre_variable, MAX_NOMBRE - 1);
        variables_archivo[idx].nombre[MAX_NOMBRE - 1] = '\0';
        variables_archivo[idx].nombre[MAX_NOMBRE - 1] = '\0';
    } else {
        strncpy(variables_archivo[idx].nombre, nombre_archivo, MAX_NOMBRE - 1);
        variables_archivo[idx].nombre[MAX_NOMBRE - 1] = '\0';
        variables_archivo[idx].nombre[MAX_NOMBRE - 1] = '\0';
    }
    
    variables_archivo[idx].archivo = archivo;
    variables_archivo[idx].modo = idx;
    return idx;
}

/* ABRIRARCHIVO */
void procesar_abrirarchivo(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    char nombre_archivo[MAX_LINEA] = "";
    char modo_str[20] = "";
    
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    if (*ptr == '$') ptr++;
    while (*ptr && *ptr != ',' && i < MAX_NOMBRE - 1) nombre_var[i++] = *ptr++;
    nombre_var[i] = '\0';
    
    const char *coma = strchr(ptr, ',');
    if (!coma) {
        fprintf(stderr, "Error: ABRIRARCHIVO requiere (variable, \"ruta\", MODO).\n");
        return;
    }
    coma++;
    while (*coma == ' ' || *coma == '\t') coma++;
    
    if (*coma != '"') {
        fprintf(stderr, "Error: ABRIRARCHIVO requiere ruta entre comillas.\n");
        return;
    }
    coma++;
    i = 0;
    while (*coma && *coma != '"' && i < MAX_LINEA - 1) nombre_archivo[i++] = *coma++;
    nombre_archivo[i] = '\0';
    
    coma++;
    char *modo_ptr = strstr(coma, "ESCRITURA");
    if (modo_ptr) strcpy(modo_str, "ESCRITURA");
    else {
        modo_ptr = strstr(coma, "AGREGAR");
        if (modo_ptr) strcpy(modo_str, "AGREGAR");
        else {
            modo_ptr = strstr(coma, "LECTURA");
            if (modo_ptr) strcpy(modo_str, "LECTURA");
            else {
                modo_ptr = strstr(coma, "LECTOESCRITURA");
                if (modo_ptr) strcpy(modo_str, "LECTOESCRITURA");
            }
        }
    }
    
    if (strlen(modo_str) == 0) {
        fprintf(stderr, "Error: Modo debe ser ESCRITURA, AGREGAR, LECTURA o LECTOESCRITURA.\n");
        return;
    }
    
    procesar_usararchivo(nombre_archivo, modo_str, nombre_var);
}

/* ESCRIBIRARCHIVO */
void procesar_escribirarchivo(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr && *ptr != ',' && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_var[i++] = *ptr++;
    }
    nombre_var[i] = '\0';
    
    const char *coma = strchr(argumento, ',');
    if (!coma) {
        fprintf(stderr, "Error: ESCRIBIRARCHIVO requiere contenido.\n");
        return;
    }
    coma++;
    while (*coma == ' ' || *coma == '\t') coma++;
    
    if (*coma != '"') {
        fprintf(stderr, "Error: ESCRIBIRARCHIVO requiere texto entre comillas.\n");
        return;
    }
    coma++;
    
    char contenido[MAX_LINEA] = "";
    i = 0;
    while (*coma && *coma != '"' && i < MAX_LINEA - 2) {
        if (*coma == '\\' && *(coma+1)) {
            coma++;
            switch (*coma) {
                case 'n': contenido[i++] = '\n'; break;
                case 't': contenido[i++] = '\t'; break;
                case '\\': contenido[i++] = '\\'; break;
                case '"': contenido[i++] = '"'; break;
                default: contenido[i++] = *coma; break;
            }
        } else {
            contenido[i++] = *coma;
        }
        coma++;
    }
    
    contenido[i++] = '\n';
    contenido[i] = '\0';
    
    for (int k = 0; k < MAX_VARS_ARCHIVO; k++) {
        if (variables_archivo[k].archivo != NULL && 
            strcmp(variables_archivo[k].nombre, nombre_var) == 0) {
            fprintf(variables_archivo[k].archivo, "%s", contenido);
            return;
        }
    }
    
    fprintf(stderr, "Error: Archivo '%s' no está abierto.\n", nombre_var);
}

/* LEERARCHIVO */
void procesar_leerarchivo(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var_archivo[MAX_NOMBRE] = "";
    char nombre_var_destino[MAX_NOMBRE] = "";
    
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr && *ptr != ',' && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_var_archivo[i++] = *ptr++;
    }
    nombre_var_archivo[i] = '\0';
    limpiar_string(nombre_var_archivo);
    
    const char *coma = strchr(argumento, ',');
    if (!coma) {
        fprintf(stderr, "Error: LEERARCHIVO requiere variable destino.\n");
        return;
    }
    
    coma++;
    i = 0;
    while (*coma == ' ' || *coma == '\t') coma++;
    while (*coma && *coma != ')' && i < MAX_NOMBRE - 1) {
        if (*coma == '$') coma++;
        else nombre_var_destino[i++] = *coma++;
    }
    nombre_var_destino[i] = '\0';
    limpiar_string(nombre_var_destino);
    
    int idx_arch = -1;
    for (int k = 0; k < MAX_VARS_ARCHIVO; k++) {
        if (variables_archivo[k].archivo != NULL && 
            strcmp(variables_archivo[k].nombre, nombre_var_archivo) == 0) {
            idx_arch = k;
            break;
        }
    }
    
    if (idx_arch < 0) {
        fprintf(stderr, "Error: Archivo '%s' no está abierto.\n", nombre_var_archivo);
        return;
    }
    
    char buffer[MAX_LINEA];
    if (fgets(buffer, MAX_LINEA, variables_archivo[idx_arch].archivo) == NULL) {
        int idx_txt = buscar_texto_var(nombre_var_destino);
        if (idx_txt >= 0) texto_vars[idx_txt].valor[0] = '\0';
        return;
    }
    
    buffer[strcspn(buffer, "\n\r")] = '\0';
    
    int idx_txt = buscar_texto_var(nombre_var_destino);
    if (idx_txt >= 0) {
        strncpy(texto_vars[idx_txt].valor, buffer, MAX_TEXTO_LEN - 1);
        texto_vars[idx_txt].valor[MAX_TEXTO_LEN - 1] = '\0';
        texto_vars[idx_txt].valor[MAX_TEXTO_LEN - 1] = '\0';
    }
}

/* LEERLINEA */
void procesar_leerlinea(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr && *ptr != ',' && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_var[i++] = *ptr++;
    }
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    int idx = buscar_variable_archivo(nombre_var);
    if (idx < 0) {
        fprintf(stderr, "Error: Variable de archivo '$%s' no declarada.\n", nombre_var);
        return;
    }
    
    const char *destino = strchr(argumento, ',');
    if (!destino) {
        fprintf(stderr, "Error: LEERLINEA requiere variable destino.\n");
        return;
    }
    destino++;
    while (*destino == ' ' || *destino == '\t') destino++;
    
    char nombre_destino[MAX_NOMBRE] = "";
    i = 0;
    ptr = destino;
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    if (*ptr == '$') ptr++;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_destino[i++] = *ptr++;
    nombre_destino[i] = '\0';
    
    char linea[MAX_TEXTO_LEN];
    if (!fgets(linea, MAX_TEXTO_LEN, variables_archivo[idx].archivo)) {
        int id = buscar_texto_var(nombre_destino);
        if (id >= 0) texto_vars[id].valor[0] = '\0';
        return;
    }
    
    size_t len = strlen(linea);
    if (len > 0 && linea[len-1] == '\n') linea[len-1] = '\0';
    
    int id = buscar_texto_var(nombre_destino);
    if (id >= 0) {
        strncpy(texto_vars[id].valor, linea, MAX_TEXTO_LEN - 1);
        texto_vars[id].valor[MAX_TEXTO_LEN - 1] = '\0';
        return;
    }
    agregar_texto_var(nombre_destino, linea);
}

/* FINARCHIVO */
int procesar_finarchivo(const char *argumento) {
    if (!argumento) return 0;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr && *ptr != ')' && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_var[i++] = *ptr++;
    }
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    for (int k = 0; k < MAX_VARS_ARCHIVO; k++) {
        if (variables_archivo[k].archivo != NULL && 
            strcmp(variables_archivo[k].nombre, nombre_var) == 0) {
            return feof(variables_archivo[k].archivo) ? 1 : 0;
        }
    }
    
    return 0;
}

/* CERRARARCHIVO */
void procesar_cerrararchivo(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr && *ptr != ')' && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_var[i++] = *ptr++;
    }
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    int idx = buscar_variable_archivo(nombre_var);
    if (idx < 0) {
        fprintf(stderr, "Error: Variable de archivo '$%s' no declarada.\n", nombre_var);
        return;
    }
    
    if (variables_archivo[idx].archivo) {
        fclose(variables_archivo[idx].archivo);
        variables_archivo[idx].archivo = NULL;
    }
}

/* FUNCIONES DE TEXTO */
void procesar_funcion_texto(const char *linea) {
    char linea_local[MAX_LINEA];
    strncpy(linea_local, linea, MAX_LINEA - 1);
    linea_local[MAX_LINEA - 1] = '\0';
    limpiar_string(linea_local);
    
    char *apertura = strchr(linea_local, '(');
    char *cierre = strrchr(linea_local, ')');
    if (!apertura || !cierre) {
        fprintf(stderr, "Error: Función de texto requiere paréntesis.\n");
        return;
    }
    
    char nombre_func[MAX_NOMBRE];
    int i = 0;
    char *p = linea_local;
    while (*p && *p != '(' && i < MAX_NOMBRE - 1) {
        if (*p == '$') p++;
        else nombre_func[i++] = *p++;
    }
    nombre_func[i] = '\0';
    limpiar_string(nombre_func);
   
    char *args[MAX_PARAMETROS];  
    int num_args = 0;            
    p = apertura + 1;
    int dentro_comillas = 0;
    char tipo_comilla = 0;
    
    while (*p && *p != ')' && num_args < MAX_PARAMETROS) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == ')') break;
        
        char arg[MAX_LINEA] = "";
        i = 0;
        int nivel = 0;
        dentro_comillas = 0;
        tipo_comilla = 0;
        
        while (*p && (nivel > 0 || !((*p == ',' || *p == ')') && !dentro_comillas)) && i < MAX_LINEA - 1) {
            if ((*p == '"' || *p == '\'') && *(p-1) != '\\') {
                if (!dentro_comillas) { dentro_comillas = 1; tipo_comilla = *p; }
                else if (*p == tipo_comilla) { dentro_comillas = 0; }
            }
            else if (!dentro_comillas) {
                if (*p == '(') nivel++;
                else if (*p == ')') nivel--;
            }
            arg[i++] = *p++;
        }
        arg[i] = '\0';
        limpiar_string(arg);
        if (strlen(arg) > 0) args[num_args++] = strdup(arg);
        if (*p == ',') p++;
    } 
   
    if (ejecutar_comando_cadena(nombre_func, args, num_args) < 0) {
        fprintf(stderr, "Error: Función de texto '%s' no reconocida o argumentos inválidos.\n", nombre_func);
    }
    
    for (int k = 0; k < num_args; k++) {
        if (args[k]) free(args[k]);
    }
}

/* PROCESAR CALCULAR */
void procesar_calcular(const char *linea) {
    if (comienza_con(linea, "COPIARTEXTO(") || comienza_con(linea, "CONCATENARTEXTO(") ||
        comienza_con(linea, "MAYUSCULAS(") || comienza_con(linea, "MINUSCULAS(") ||
        comienza_con(linea, "RECORTARTEXTO(") || comienza_con(linea, "REEMPLAZARTEXTO(") ||
        comienza_con(linea, "ENTEROATEXTO(") || comienza_con(linea, "DECIMALATEXTO(") ||
        comienza_con(linea, "CARACTERATEXTO(") || comienza_con(linea, "REPETIRTEXTO(") ||
        comienza_con(linea, "EXTRAERTEXTO(") || comienza_con(linea, "DIVIDIRTEXTO(")) {
        procesar_funcion_texto(linea);
        return;
    }
    
    char linea_local[MAX_LINEA];
    strncpy(linea_local, linea, MAX_LINEA - 1);
    linea_local[MAX_LINEA - 1] = '\0';
    limpiar_string(linea_local);
    
    const char *ptr = strchr(linea_local, '$');
    if (!ptr) {
        fprintf(stderr, "Error: No se encontró variable '$' en la línea.\n");
        return;
    }

    const char *igual = strchr(ptr, '=');
    
    if (!igual) {
        fprintf(stderr, "Error: Formato de asignación inválido (falta '=').\n");
        return;
    }
    
    char lado_izq[MAX_LINEA];
    int len_izq = igual - ptr;
    if (len_izq <= 0 || len_izq >= MAX_LINEA) {
        fprintf(stderr, "Error: Formato de asignación inválido.\n");
        return;
    }
    strncpy(lado_izq, ptr, len_izq);
    lado_izq[len_izq] = '\0';

    if (strchr(lado_izq, '[')) {
        char *start = lado_izq;
        char *end = lado_izq + strlen(lado_izq) - 1;
        while (*start == ' ' || *start == '\t') start++;
        while (end > start && (*end == ' ' || *end == '\t')) *end-- = '\0';
        if (start != lado_izq) {
            memmove(lado_izq, start, strlen(start) + 1);
        }
    } else {
        limpiar_string(lado_izq);
    }

    char nombre_dest[MAX_NOMBRE] = "";
    const char *p_nombre = lado_izq + 1;
    int i = 0;
    while (es_alnum(*p_nombre) && i < MAX_NOMBRE - 1) nombre_dest[i++] = *p_nombre++;
    nombre_dest[i] = '\0';
   
    igual++;
    while (*igual == ' ' || *igual == '\t') igual++;
    char expresion[MAX_LINEA];
    strncpy(expresion, igual, MAX_LINEA - 1);
    expresion[MAX_LINEA - 1] = '\0';
    limpiar_string(expresion);
    
    int exito_expr;
    double resultado = evaluar_expresion_completa(expresion, &exito_expr);
    if (!exito_expr) {
        return;
    }
    
    const char *corchete1 = strchr(lado_izq, '[');
    if (corchete1) {
        const char *cierre1 = strchr(corchete1 + 1, ']');
        const char *corchete2 = cierre1 ? strchr(cierre1 + 1, '[') : NULL;
        
        if (corchete2) {
            char fila_str[MAX_LINEA] = "", columna_str[MAX_LINEA] = "";
            
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
                if (nivel > 0) columna_str[i++] = *p_cor2;
                p_cor2++;
            }
            columna_str[i] = '\0'; limpiar_string(columna_str);
            
            int efila, ecol;
            double vfila = evaluar_expresion_completa(fila_str, &efila);
            double vcol = evaluar_expresion_completa(columna_str, &ecol);
            int fila = efila ? (int)vfila : atoi(fila_str);
            int col = ecol ? (int)vcol : atoi(columna_str);

            double val_var;
                    
            for (int sc = scope_actual; sc >= 0; sc--) {
                ScopeLocal *scp = &scopes_locales[sc];
                for (int v = 0; v < scp->num_variables; v++) {
                    if (strcmp(scp->variables[v].nombre, nombre_dest) == 0) {
                        switch(scp->variables[v].tipo) {
                            case 0: val_var = (double)scp->variables[v].valor.valor_entero; break;
                            case 1: val_var = (double)scp->variables[v].valor.valor_sin_signo; break;
                            case 2: case 3: val_var = scp->variables[v].valor.valor_decimal; break;
                            case 4: val_var = (double)scp->variables[v].valor.valor_caracter; break;
                            case 5: val_var = (double)scp->variables[v].valor.valor_caracter_sin_signo; break;
                            default: val_var = 0; break;
                        }
                        
                        if (val_var >= 2000000.0 && val_var < 2060000.0) {
                            int ref = (int)(val_var - 2000000.0);
                            int tipo_mat = ref / 10000;
                            int idx_pool = ref % 10000;
                                                    
                            int filas_max = 0, cols_max = 0;
                            switch(tipo_mat) {
                                case 0: filas_max = matrices_enteras[idx_pool].filas; cols_max = matrices_enteras[idx_pool].columnas; break;
                                case 1: case 3: filas_max = matrices_decimales[idx_pool].filas; cols_max = matrices_decimales[idx_pool].columnas; break;
                                case 2: filas_max = matrices_enteras_sin_signo[idx_pool].filas; cols_max = matrices_enteras_sin_signo[idx_pool].columnas; break;
                                case 4: filas_max = matrices_caracter[idx_pool].filas; cols_max = matrices_caracter[idx_pool].columnas; break;
                                case 5: filas_max = matrices_caracter_sin_signo[idx_pool].filas; cols_max = matrices_caracter_sin_signo[idx_pool].columnas; break;
                            }
                            
                            if (fila < 0 || fila >= filas_max || col < 0 || col >= cols_max) {
                                return;
                            }
                            
                            switch(tipo_mat) {
                                case 0: 
                                    matrices_enteras[idx_pool].valores[fila][col] = (int)resultado; 
                                    break;
                                case 1: case 3: 
                                    matrices_decimales[idx_pool].valores[fila][col] = resultado; 
                                    break;
                                case 2: 
                                    matrices_enteras_sin_signo[idx_pool].valores[fila][col] = (unsigned int)resultado; 
                                    break;
                                case 4: 
                                    matrices_caracter[idx_pool].valores[fila][col] = (char)resultado; 
                                    break;
                                case 5: 
                                    matrices_caracter_sin_signo[idx_pool].valores[fila][col] = (unsigned char)resultado; 
                                    break;
                            }
                            return;
                        }
                        goto buscar_local_o_global_mat;
                    }
                }
            }
            
            buscar_local_o_global_mat:;
            
            int tipo_loc = -1, idx_loc = -1;
            if (buscar_matriz_local(nombre_dest, &tipo_loc, &idx_loc)) {
                int filas_max = 0, cols_max = 0;
                switch(tipo_loc) {
                    case 0: filas_max = matrices_enteras[idx_loc].filas; cols_max = matrices_enteras[idx_loc].columnas; break;
                    case 1: case 3: filas_max = matrices_decimales[idx_loc].filas; cols_max = matrices_decimales[idx_loc].columnas; break;
                    case 2: filas_max = matrices_enteras_sin_signo[idx_loc].filas; cols_max = matrices_enteras_sin_signo[idx_loc].columnas; break;
                    case 4: filas_max = matrices_caracter[idx_loc].filas; cols_max = matrices_caracter[idx_loc].columnas; break;
                    case 5: filas_max = matrices_caracter_sin_signo[idx_loc].filas; cols_max = matrices_caracter_sin_signo[idx_loc].columnas; break;
                }
                if (fila < 0 || fila >= filas_max || col < 0 || col >= cols_max) {
                    return;
                }
                
                switch(tipo_loc) {
                    case 0: matrices_enteras[idx_loc].valores[fila][col] = (int)resultado; return;
                    case 1: case 3: matrices_decimales[idx_loc].valores[fila][col] = resultado; return;
                    case 2: matrices_enteras_sin_signo[idx_loc].valores[fila][col] = (unsigned int)resultado; return;
                    case 4: matrices_caracter[idx_loc].valores[fila][col] = (char)resultado; return;
                    case 5: matrices_caracter_sin_signo[idx_loc].valores[fila][col] = (unsigned char)resultado; return;
                }
            }
           
            if (buscar_matriz_entera(nombre_dest) >= 0) { 
                set_matriz_entera_valor(nombre_dest, fila, col, (int)resultado); return; 
            }

            if (buscar_matriz_decimal(nombre_dest) >= 0) { 
                set_matriz_decimal_valor(nombre_dest, fila, col, resultado); return; 
            }

            if (buscar_matriz_entera_sin_signo(nombre_dest) >= 0) { 
                set_matriz_entera_sin_signo_valor(nombre_dest, fila, col, (unsigned int)resultado); return; 
            }

            if (buscar_matriz_decimal_sin_signo(nombre_dest) >= 0) { 
                set_matriz_decimal_sin_signo_valor(nombre_dest, fila, col, resultado); return; 
            }

            if (buscar_matriz_caracter(nombre_dest) >= 0) { 
                set_matriz_caracter_valor(nombre_dest, fila, col, (char)resultado); return; 
            }

            if (buscar_matriz_caracter_sin_signo(nombre_dest) >= 0) { 
                set_matriz_caracter_sin_signo_valor(nombre_dest, fila, col, (unsigned char)resultado); return; 
            }

            fprintf(stderr, "Error: Matriz '$%s' no declarada.\n", nombre_dest);
            return;
        }
    }    

    corchete1 = strchr(lado_izq, '[');
    if (corchete1) {
        char indice_str[MAX_LINEA] = "";
        const char *p_cor = corchete1 + 1;
        i = 0; int nivel = 1;
        while (*p_cor && nivel > 0 && i < MAX_LINEA - 1) {
            if (*p_cor == '[') nivel++; else if (*p_cor == ']') nivel--;
            if (nivel > 0) indice_str[i++] = *p_cor;
            p_cor++;
        }
        indice_str[i] = '\0'; limpiar_string(indice_str);
        
        int eindice;
        double vindice = evaluar_expresion_completa(indice_str, &eindice);
        int indice = eindice ? (int)vindice : atoi(indice_str);
        
        double val_param = 0;
        
        for (int sc = scope_actual; sc >= 0; sc--) {
            ScopeLocal *scp = &scopes_locales[sc];
            for (int v = 0; v < scp->num_variables; v++) {
                if (strcmp(scp->variables[v].nombre, nombre_dest) == 0) {
                    switch(scp->variables[v].tipo) {
                        case 0: val_param = (double)scp->variables[v].valor.valor_entero; break;
                        case 1: val_param = (double)scp->variables[v].valor.valor_sin_signo; break;
                        case 2: case 3: val_param = scp->variables[v].valor.valor_decimal; break;
                        case 4: val_param = (double)scp->variables[v].valor.valor_caracter; break;
                        case 5: val_param = (double)scp->variables[v].valor.valor_caracter_sin_signo; break;
                        default: val_param = 0; break;
                    }
                    if (val_param >= 1000000.0 && val_param < 1060000.0) {
                        int ref = (int)(val_param - 1000000.0);
                        int tipo_list = ref / 10000;
                        int idx_pool = ref % 10000;
                        switch(tipo_list) {
                            case 0: listas_enteras[idx_pool].valores[indice] = (int)resultado; return;
                            case 1: case 3: listas_decimales[idx_pool].valores[indice] = resultado; return;
                            case 2: listas_enteras_sin_signo[idx_pool].valores[indice] = (unsigned int)resultado; return;
                            case 4: listas_caracter[idx_pool].valores[indice] = (char)resultado; return;
                            case 5: listas_caracter_sin_signo[idx_pool].valores[indice] = (unsigned char)resultado; return;
                        }
                    }
                    goto buscar_local_o_global_list;
                }
            }
        }
        
        buscar_local_o_global_list:;
        int tipo_loc = -1, idx_loc = -1;
        if (buscar_lista_local(nombre_dest, &tipo_loc, &idx_loc)) {
            switch(tipo_loc) {
                case 0: listas_enteras[idx_loc].valores[indice] = (int)resultado; return;
                case 1: case 3: listas_decimales[idx_loc].valores[indice] = resultado; return;
                case 2: listas_enteras_sin_signo[idx_loc].valores[indice] = (unsigned int)resultado; return;
                case 4: listas_caracter[idx_loc].valores[indice] = (char)resultado; return;
                case 5: listas_caracter_sin_signo[idx_loc].valores[indice] = (unsigned char)resultado; return;
            }
        }
        
        if (buscar_lista_entera(nombre_dest) >= 0) { set_lista_entera_valor(nombre_dest, indice, (int)resultado); return; }
        if (buscar_lista_decimal(nombre_dest) >= 0) { set_lista_decimal_valor(nombre_dest, indice, resultado); return; }
        if (buscar_lista_entera_sin_signo(nombre_dest) >= 0) { set_lista_entera_sin_signo_valor(nombre_dest, indice, (unsigned int)resultado); return; }
        if (buscar_lista_decimal_sin_signo(nombre_dest) >= 0) { set_lista_decimal_sin_signo_valor(nombre_dest, indice, resultado); return; }
        if (buscar_lista_caracter(nombre_dest) >= 0) { set_lista_caracter_valor(nombre_dest, indice, (char)resultado); return; }
        if (buscar_lista_caracter_sin_signo(nombre_dest) >= 0) { set_lista_caracter_sin_signo_valor(nombre_dest, indice, (unsigned char)resultado); return; }
        
        fprintf(stderr, "Error: Lísta '$%s' no declarada.\n", nombre_dest);
        return;
    }
    
    if (scope_actual >= 0) {
        for (int sc = scope_actual; sc >= 0; sc--) {
            ScopeLocal *scp = &scopes_locales[sc];
            for (int j = 0; j < scp->num_variables; j++) {
                if (strcmp(scp->variables[j].nombre, nombre_dest) == 0) {
                    switch (scp->variables[j].tipo) {
                        case 0: scp->variables[j].valor.valor_entero = (int)resultado; break;
                        case 1: scp->variables[j].valor.valor_sin_signo = (unsigned int)resultado; break;
                        case 2: case 3: scp->variables[j].valor.valor_decimal = resultado; break;
                        case 4: scp->variables[j].valor.valor_caracter = (char)resultado; break;
                        case 5: scp->variables[j].valor.valor_caracter_sin_signo = (unsigned char)resultado; break;
                    }
                    return;
                }
            }
        }
    }
    
    for (int k = 0; k < num_variables; k++) if (strcmp(variables[k].nombre, nombre_dest) == 0) { variables[k].valor = (int)resultado; return; }
    for (int k = 0; k < num_variables_decimal; k++) if (strcmp(variables_decimal[k].nombre, nombre_dest) == 0) { variables_decimal[k].valor = resultado; return; }
    for (int k = 0; k < num_variables_sin_signo; k++) if (strcmp(variables_sin_signo[k].nombre, nombre_dest) == 0) { variables_sin_signo[k].valor = (unsigned int)resultado; return; }
    for (int k = 0; k < num_variables_decimal_sin_signo; k++) if (strcmp(variables_decimal_sin_signo[k].nombre, nombre_dest) == 0) { variables_decimal_sin_signo[k].valor = resultado; return; }
    for (int k = 0; k < num_variables_caracter; k++) if (strcmp(variables_caracter[k].nombre, nombre_dest) == 0) { variables_caracter[k].valor = (char)resultado; return; }
    for (int k = 0; k < num_variables_caracter_sin_signo; k++) if (strcmp(variables_caracter_sin_signo[k].nombre, nombre_dest) == 0) { variables_caracter_sin_signo[k].valor = (unsigned char)resultado; return; }
    
    fprintf(stderr, "Error: Variable '$%s' no declarada.\n", nombre_dest);
}

/* PROCESAR ESCRIBIR */
void procesar_escribir(const char *texto) {
    if (!texto) return;
    int dec_precisions[MAX_VARS];
    int dec_count = 0;
    int dec_actual = 0;

    char buffer[MAX_LINEA];
    strncpy(buffer, texto, MAX_LINEA - 1);
    buffer[MAX_LINEA - 1] = '\0';
    buffer[MAX_LINEA - 1] = '\0';
    
    char *ptr = buffer;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr != '(') {
        fprintf(stderr, "Error: ESCRIBIR requiere paréntesis de apertura.\n");
        return;
    }
    ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr != '"') {
        fprintf(stderr, "Error: ESCRIBIR requiere texto entre comillas.\n");
        return;
    }
    ptr++;
    
    char texto_final[MAX_LINEA] = "";
    int i = 0;
    while (*ptr && *ptr != '"' && i < MAX_LINEA - 1) texto_final[i++] = *ptr++;
    texto_final[i] = '\0';
    
    if (*ptr != '"') {
        fprintf(stderr, "Error: ESCRIBIR requiere comilla de cierre.\n");
        return;
    }
    ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr == ',') {
        char *dec_ptr = ptr + 1;
        while (*dec_ptr == ' ' || *dec_ptr == '\t') dec_ptr++;
        if (strncmp(dec_ptr, "DECIMALES", 9) == 0) {
            dec_ptr += 9;
            while (*dec_ptr == ' ' || *dec_ptr == '\t') dec_ptr++;
            if (*dec_ptr == '(') {
                dec_ptr++;
                while (*dec_ptr && *dec_ptr != ')' && dec_count < MAX_VARS) {
                    while (*dec_ptr == ' ' || *dec_ptr == '\t' || *dec_ptr == ',') dec_ptr++;
                    if (*dec_ptr == ')') break;
                    char num_str[16] = "";
                    int i = 0;
                    while (*dec_ptr >= '0' && *dec_ptr <= '9' && i < 15) num_str[i++] = *dec_ptr++;
                    num_str[i] = '\0';
                    if (i > 0) dec_precisions[dec_count++] = atoi(num_str);
                }
                if (*dec_ptr == ')') dec_ptr++;
                ptr = dec_ptr;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
            }
        }
    }    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != ')') {
        fprintf(stderr, "Error: ESCRIBIR requiere paréntesis de cierre.\n");
        return;
    }
    ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    int tiene_salto = (strncmp(ptr, "SALTO", 5) == 0) ? 1 : 0;
    
    /* PROCESAR OUTPUT */
    char resultado[MAX_LINEA] = "";
    ptr = texto_final;
    
    while (*ptr) {
        if (*ptr == '[') {
            const char *cierre = NULL;
            int nivel = 1;
            const char *temp_scan = ptr + 1;
            
            while (*temp_scan) {
                if (*temp_scan == '[') nivel++;
                else if (*temp_scan == ']') {
                    nivel--;
                    if (nivel == 0) {
                        cierre = temp_scan;
                        break;
                    }
                }
                temp_scan++;
            }

            if (cierre) {
                char contenido[MAX_LINEA] = "";
                int j = 0;
                const char *temp = ptr + 1;
                while (temp < cierre && j < MAX_LINEA - 1) contenido[j++] = *temp++;
                contenido[j] = '\0';
                limpiar_string(contenido);
                
                int parece_expresion = 0;
                for (int k = 0; contenido[k]; k++) {
                    if ((contenido[k] >= '0' && contenido[k] <= '9') ||
                        contenido[k] == '$' ||
                        contenido[k] == '+' || contenido[k] == '-' ||
                        contenido[k] == '*' || contenido[k] == '/' ||
                        contenido[k] == '(' || contenido[k] == ')') {
                        parece_expresion = 1;
                        break;
                    }
                }
                if (parece_expresion) {
                    int exito;
                    double valor = evaluar_expresion_completa(contenido, &exito);
                    if (exito) {
                        if (valor >= 3000000.0 && valor < 3100000.0) {
                            int idx_txt = (int)(valor - 3000000.0);
                            if (idx_txt >= 0 && idx_txt < num_texto_vars) {
                                strcat(resultado, texto_vars[idx_txt].valor);
                            }
                            ptr = (char *)cierre + 1;
                            continue;
                        }
                        char buf[64];
                        if (valor == (double)(long long)valor && fabs(valor) < 9007199254740992.0) {
                            sprintf(buf, "[%lld]", (long long)valor);
                        } else {
                            int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6;
                            dec_actual++;
                            sprintf(buf, "[%.*f]", p, valor);
                        }
                        strcat(resultado, buf);
                        ptr = (char *)cierre + 1;
                        continue;
                    }
                }
                
            } else {
                char temp_char[2] = {'[', '\0'};
                strcat(resultado, temp_char);
            }
            ptr++;
            continue;
        }
        
        if (*ptr == '\\' && *(ptr+1) == '$') { strcat(resultado, "$"); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == 'n') { strcat(resultado, "\n"); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == 't') { strcat(resultado, "\t"); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == '\\') { strcat(resultado, "\\"); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == '[') { strcat(resultado, "["); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == ']') { strcat(resultado, "]"); ptr += 2; continue; }
        
        if (*ptr == '$') {
            ptr++;
            char nombre[MAX_NOMBRE];
            int j = 0;
            while (es_alnum(*ptr) && j < MAX_NOMBRE - 1) nombre[j++] = *ptr++;
            nombre[j] = '\0';
            int encontrado = 0;
            char buf_num[64] = {0};
           // char buf_texto[MAX_TEXTO_LEN] = {0};
            int idx;

            char *corchete = strchr(ptr, '[');
            if (corchete) {
                char indice_str[MAX_LINEA];
                int k = 0;
                char *p = corchete + 1;
                int nivel = 1;
                while (*p && nivel > 0 && k < MAX_LINEA - 1) {
                    if (*p == '[') nivel++;
                    else if (*p == ']') nivel--;
                    if (nivel > 0) indice_str[k++] = *p;
                    p++;
                }
                indice_str[k] = '\0';
                limpiar_string(indice_str);
                            
                int idx_lista = -1;    

                int tipo_local = -1, idx_local = -1;
                int encontrado_local = (scope_actual >= 0) ? buscar_lista_local(nombre, &tipo_local, &idx_local) : 0;

                if (encontrado_local) {
                    int exito_indice;
                    double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);

                    switch(tipo_local) {
                        case 0: sprintf(buf_num, "%lld", (long long)listas_enteras[idx_local].valores[indice]); break;
                        case 1: { int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; dec_actual++; sprintf(buf_num, "%.*f", p, listas_decimales[idx_local].valores[indice]); break; }
                        case 2: sprintf(buf_num, "%llu", (unsigned long long)listas_enteras_sin_signo[idx_local].valores[indice]); break;
                        case 3: { int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; dec_actual++; sprintf(buf_num, "%.*f", p, listas_decimales_sin_signo[idx_local].valores[indice]); break; }
                        case 4: sprintf(buf_num, "%c", (unsigned char)listas_caracter[idx_local].valores[indice]); break;
                        case 5: sprintf(buf_num, "%c", listas_caracter_sin_signo[idx_local].valores[indice]); break;
                        default: sprintf(buf_num, "?"); break;
                    }
                    encontrado = 1;
                }

                else if (buscar_lista_entera(nombre) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    sprintf(buf_num, "%lld", (long long)get_lista_entera_valor(nombre, indice)); encontrado = 1;
                }

                else if ((idx_lista = buscar_lista_decimal(nombre)) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; dec_actual++;
                    sprintf(buf_num, "%.*f", p, get_lista_decimal_valor(nombre, indice)); encontrado = 1;
                }

                else if ((idx_lista = buscar_lista_entera_sin_signo(nombre)) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    sprintf(buf_num, "%llu", (unsigned long long)get_lista_entera_sin_signo_valor(nombre, indice)); encontrado = 1;
                }

                else if ((idx_lista = buscar_lista_decimal_sin_signo(nombre)) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; dec_actual++;
                    sprintf(buf_num, "%.*f", p, get_lista_decimal_sin_signo_valor(nombre, indice)); encontrado = 1;
                }

                else if ((idx_lista = buscar_lista_caracter(nombre)) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    sprintf(buf_num, "%c", (unsigned char)get_lista_caracter_valor(nombre, indice)); encontrado = 1;
                }

                else if ((idx_lista = buscar_lista_caracter_sin_signo(nombre)) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    sprintf(buf_num, "%c", get_lista_caracter_sin_signo_valor(nombre, indice)); encontrado = 1;
                }
                
                if (!encontrado) {
                    char *cierre1 = strchr(ptr, ']');
                    char *corchete2 = cierre1 ? strchr(cierre1 + 1, '[') : NULL;
        
                    if (corchete2) {
                        char fila_str[MAX_LINEA] = "", col_str[MAX_LINEA] = "";
                        int k = 0, nivel = 1;
                        const char *p1 = ptr + 1;
                        while (*p1 && nivel > 0 && k < MAX_LINEA - 1) {
                            if (*p1 == '[') nivel++; else if (*p1 == ']') nivel--;
                            if (nivel > 0) fila_str[k++] = *p1;
                            p1++;
                        }
                        fila_str[k] = '\0'; limpiar_string(fila_str);
        
                        k = 0; nivel = 1;
                        const char *p2 = corchete2 + 1;
                        while (*p2 && nivel > 0 && k < MAX_LINEA - 1) {
                            if (*p2 == '[') nivel++; else if (*p2 == ']') nivel--;
                            if (nivel > 0) col_str[k++] = *p2;
                            p2++;
                        }
                        col_str[k] = '\0'; limpiar_string(col_str);
        
                        nivel = 1; p2 = corchete2 + 1;
                        while (*p2 && nivel > 0) { if (*p2=='[') nivel++; else if (*p2==']') nivel--; p2++; }
        
                        int tipo_var; double val_var;
                        int found_as_param = buscar_variable_local(nombre, &tipo_var, &val_var);
        
                        if (found_as_param && val_var >= 2000000.0 && val_var < 2060000.0) {
                            int ref = (int)(val_var - 2000000.0);
                            int tipo_mat = ref / 10000;
                            int idx_pool = ref % 10000;
            
                            int ef, ec; 
                            double vf = evaluar_expresion_completa(fila_str, &ef); 
                            double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); 
                            int c = ec ? (int)vc : atoi(col_str);
            
                            switch(tipo_mat) {
                                case 0: sprintf(buf_num, "%lld", (long long)matrices_enteras[idx_pool].valores[f][c]); break;
                                case 1: case 3: { 
                                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; 
                                    dec_actual++; 
                                    sprintf(buf_num, "%.*f", p, matrices_decimales[idx_pool].valores[f][c]); 
                                    break; 
                                }
                                case 2: sprintf(buf_num, "%llu", (unsigned long long)matrices_enteras_sin_signo[idx_pool].valores[f][c]); break;
                                case 4: sprintf(buf_num, "%c", (unsigned char)matrices_caracter[idx_pool].valores[f][c]); break;
                                case 5: sprintf(buf_num, "%c", matrices_caracter_sin_signo[idx_pool].valores[f][c]); break;
                            }
                            encontrado = 1; 
                            ptr = (char *)p2; 
                            strcat(resultado, buf_num); 
                            continue;
                        }
        
                        int tipo_loc = -1, idx_loc = -1;
                        int found_loc = buscar_matriz_local(nombre, &tipo_loc, &idx_loc);
        
                        if (found_loc) {
                            int ef, ec; 
                            double vf = evaluar_expresion_completa(fila_str, &ef); 
                            double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); 
                            int c = ec ? (int)vc : atoi(col_str);
            
                            switch(tipo_loc) {
                                case 0: sprintf(buf_num, "%lld", (long long)matrices_enteras[idx_loc].valores[f][c]); break;
                                case 1: case 3: { 
                                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; 
                                    dec_actual++; 
                                    sprintf(buf_num, "%.*f", p, matrices_decimales[idx_loc].valores[f][c]); 
                                    break; 
                                }
                                case 2: sprintf(buf_num, "%llu", (unsigned long long)matrices_enteras_sin_signo[idx_loc].valores[f][c]); break;
                                case 4: sprintf(buf_num, "%c", (unsigned char)matrices_caracter[idx_loc].valores[f][c]); break;
                                case 5: sprintf(buf_num, "%c", matrices_caracter_sin_signo[idx_loc].valores[f][c]); break;
                            }
                            encontrado = 1; 
                            ptr = (char *)p2; 
                            strcat(resultado, buf_num); 
                            continue;
                        }
        
                        if (buscar_matriz_entera(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            sprintf(buf_num, "%lld", (long long)get_matriz_entera_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else if (buscar_matriz_decimal(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; dec_actual++;
                            sprintf(buf_num, "%.*f", p, get_matriz_decimal_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else if (buscar_matriz_entera_sin_signo(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            sprintf(buf_num, "%llu", (unsigned long long)get_matriz_entera_sin_signo_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else if (buscar_matriz_decimal_sin_signo(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; dec_actual++;
                            sprintf(buf_num, "%.*f", p, get_matriz_decimal_sin_signo_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else if (buscar_matriz_caracter(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            sprintf(buf_num, "%c", (unsigned char)get_matriz_caracter_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else if (buscar_matriz_caracter_sin_signo(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            sprintf(buf_num, "%c", get_matriz_caracter_sin_signo_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }
                    }
                }           
                if (encontrado) {
                    strcat(resultado, buf_num);
                    ptr = p;
                    if (*ptr == ']') ptr++;
                    continue;
                }
            }

            int idx_txt = -1;
            if (scope_actual >= 0) {
                for (int s = scope_actual; s >= 0; s--) {
                    for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                        if (strcmp(scopes_locales[s].nombres_textos[v], nombre) == 0) {
                            idx_txt = scopes_locales[s].indices_textos[v];
                            break;
                        }
                    }
                    if (idx_txt >= 0) break;
                }
            }
            if (idx_txt < 0) idx_txt = buscar_texto_var(nombre);

            if (idx_txt >= 0) {
                strcat(resultado, texto_vars[idx_txt].valor);
                encontrado = 1;
            }
            else if ((idx_txt = buscar_texto_constante(nombre)) >= 0) {
                strcat(resultado, texto_constantes[idx_txt].valor);
                encontrado = 1;
            }

            if (!encontrado) {
                for (int s = MAX_SCOPES - 1; s >= 0 && !encontrado; s--) {
                    if (scopes_locales[s].num_variables == 0) continue;
                    for (int v = 0; v < scopes_locales[s].num_variables && !encontrado; v++) {
                        if (strcmp(scopes_locales[s].variables[v].nombre, nombre) == 0) {
                            switch(scopes_locales[s].variables[v].tipo) {
                                case 0: {
                                        double val = (double)scopes_locales[s].variables[v].valor.valor_entero;
                                        if (val >= 3000000.0 && val < 3100000.0) {
                                            int idx_txt = (int)(val - 3000000.0);
                                            if (idx_txt >= 0 && idx_txt < num_texto_vars) {
                                                strcat(resultado, texto_vars[idx_txt].valor);
                                                encontrado = 1;
                                            }
                                        } else {
                                            sprintf(buf_num, "%lld", (long long)val);
                                        }
                                        break;
                                    }
                                case 1: {
                                        double val = (double)scopes_locales[s].variables[v].valor.valor_sin_signo;
                                        if (val >= 3000000.0 && val < 3100000.0) {
                                            int idx_txt = (int)(val - 3000000.0);
                                            if (idx_txt >= 0 && idx_txt < num_texto_vars) {
                                                strcat(resultado, texto_vars[idx_txt].valor);
                                                encontrado = 1;
                                            }
                                        } else {
                                            sprintf(buf_num, "%llu", (unsigned long long)val);
                                        }
                                        break;
                                    } 
                                case 2: case 3: {
                                        double val = scopes_locales[s].variables[v].valor.valor_decimal;
                                        if (val >= 3000000.0 && val < 3100000.0) {
                                            int idx_txt = (int)(val - 3000000.0);
                                            if (idx_txt >= 0 && idx_txt < num_texto_vars) {
                                                strcat(resultado, texto_vars[idx_txt].valor);
                                                encontrado = 1;
                                            }
                                        } else {
                                            double val_red = round(val);
                                            if (fabs(val - val_red) < 1e-9 && fabs(val) < 9007199254740992.0) {
                                                sprintf(buf_num, "%lld", (long long)val_red);
                                            } else {
                                                int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6;
                                                dec_actual++;
                                                sprintf(buf_num, "%.*f", p, val);
                                            }
                                        }
                                    } break;
                                case 4: sprintf(buf_num, "%c", (char)scopes_locales[s].variables[v].valor.valor_caracter); break;
                                case 5: sprintf(buf_num, "%c", (unsigned char)scopes_locales[s].variables[v].valor.valor_caracter_sin_signo); break;
                                default: sprintf(buf_num, "%g", scopes_locales[s].variables[v].valor.valor_decimal); break;
                            }
                            encontrado = 1;
                        }
                    }
                }
            }

            if (!encontrado) {
                if ((idx = buscar_variable_decimal(nombre)) >= 0) { 
                    double val = variables_decimal[idx].valor;
                    double val_red = round(val);
                    if (fabs(val - val_red) < 1e-9 && fabs(val) < 9007199254740992.0) {
                        sprintf(buf_num, "%lld", (long long)val_red);
                    } else {
                        int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6;
                        dec_actual++;
                        sprintf(buf_num, "%.*f", p, val);
                    }
                    encontrado = 1; 
                }
                else if ((idx = buscar_variable(nombre)) >= 0) { sprintf(buf_num, "%lld", (long long)variables[idx].valor); encontrado = 1; }
                else if ((idx = buscar_variable_sin_signo(nombre)) >= 0) { sprintf(buf_num, "%llu", (unsigned long long)variables_sin_signo[idx].valor); encontrado = 1; }
                else if ((idx = buscar_variable_decimal_sin_signo(nombre)) >= 0) { 
                    double val = variables_decimal_sin_signo[idx].valor;
                    double val_red = round(val);
                    if (fabs(val - val_red) < 1e-9 && fabs(val) < 9007199254740992.0) {
                        sprintf(buf_num, "%lld", (long long)val_red);
                    } else {
                        int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6;
                        dec_actual++;
                        sprintf(buf_num, "%.*f", p, val);
                    }
                    encontrado = 1; 
                }
                else if ((idx = buscar_variable_caracter(nombre)) >= 0) { sprintf(buf_num, "%c", (char)variables_caracter[idx].valor); encontrado = 1; }
                else if ((idx = buscar_variable_caracter_sin_signo(nombre)) >= 0) { sprintf(buf_num, "%c", (unsigned char)variables_caracter_sin_signo[idx].valor); encontrado = 1; }
            }

            if (encontrado) {
                if (buf_num[0] != '\0') {
                    strcat(resultado, buf_num);
                }
                continue; 
            }
            
            fprintf(stderr, "Error: Variable '$%s' no declarada.\n", nombre);
            return;
        }

        else {
            char temp[2] = {*ptr, '\0'};
            strcat(resultado, temp);
            ptr++;
        } 
    }
    
    printf("%s", resultado);
    if (tiene_salto) printf("\n");
    fflush(stdout);
}

/* PROCESAR LEER */
void procesar_leer(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr == '(') {
        ptr++;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
    }
    
    int i = 0;
    if (*ptr == '$') ptr++;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_var[i++] = *ptr++;
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    if (strlen(nombre_var) == 0) {
        fprintf(stderr, "Error: LEER requiere una variable válida.\n");
        return;
    }
    
    char entrada[MAX_LINEA];
    if (!fgets(entrada, MAX_LINEA, stdin)) return;
    limpiar_string(entrada);
    
    char *fin;
    double valor_num = strtod(entrada, &fin);
    int es_numero = (fin != entrada && *fin == '\0');
    
    int idx_pool = -1;
    if (scope_actual >= 0) {
        for (int s = scope_actual; s >= 0; s--) {
            for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                if (strcmp(scopes_locales[s].nombres_textos[v], nombre_var) == 0) {
                    idx_pool = scopes_locales[s].indices_textos[v];
                    break;
                }
            }
            if (idx_pool != -1) break;
        }
    }
    
    if (idx_pool >= 0) {
        strncpy(texto_vars[idx_pool].valor, entrada, MAX_TEXTO_LEN - 1);
        texto_vars[idx_pool].valor[MAX_TEXTO_LEN - 1] = '\0';
        return;
    }
    
    int idx = buscar_texto_var(nombre_var);
    if (idx >= 0) {
        strncpy(texto_vars[idx].valor, entrada, MAX_TEXTO_LEN - 1);
        return;
    }
    
    if (scope_actual >= 0 && en_funcion) {
        agregar_texto_local(nombre_var, entrada);
        return;
    }
    
    agregar_texto_var(nombre_var, entrada);
     
    if (idx >= 0) {
        strncpy(texto_vars[idx].valor, entrada, MAX_TEXTO_LEN - 1);
        texto_vars[idx].valor[MAX_TEXTO_LEN - 1] = '\0';
        return;
    }

    if (strlen(entrada) > 0) {
        idx = buscar_variable_caracter(nombre_var);
        if (idx >= 0) { variables_caracter[idx].valor = entrada[0]; return; }
    }
    
    if (strlen(entrada) > 0) {
        idx = buscar_variable_caracter_sin_signo(nombre_var);
        if (idx >= 0) { variables_caracter_sin_signo[idx].valor = (unsigned char)entrada[0]; return; }
    }
    
    if (es_numero) {
        idx = buscar_variable(nombre_var);
        if (idx >= 0) { variables[idx].valor = (int)valor_num; return; }
    }
    
    if (es_numero) {
        idx = buscar_variable_sin_signo(nombre_var);
        if (idx >= 0) { variables_sin_signo[idx].valor = (unsigned int)valor_num; return; }
    }
    
    idx = buscar_variable_decimal(nombre_var);
    if (idx >= 0) { variables_decimal[idx].valor = valor_num; return; }
    
    idx = buscar_variable_decimal_sin_signo(nombre_var);
    if (idx >= 0) { variables_decimal_sin_signo[idx].valor = valor_num; return; }
    
    if (en_funcion) {
        int local_idx = -1;
        if (scope_actual >= 0) {
            for (int s = scope_actual; s >= 0; s--) {
                for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                    if (strcmp(scopes_locales[s].nombres_textos[v], nombre_var) == 0) {
                        local_idx = scopes_locales[s].indices_textos[v];
                        break;
                    }
                }
                if (local_idx >= 0) break;
            }
        }
        if (local_idx >= 0) {
            strncpy(texto_vars[local_idx].valor, entrada, MAX_TEXTO_LEN - 1);
            texto_vars[local_idx].valor[MAX_TEXTO_LEN - 1] = '\0';
            return;
        }

            if (en_funcion) {
        int local_idx = -1;
        if (scope_actual >= 0 && scope_actual < MAX_SCOPES) {
            for (int s = scope_actual; s >= 0; s--) {
                for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                    const char *n1 = scopes_locales[s].nombres_textos[v];
                    const char *n2 = nombre_var;
                    if (n1[0] == '$') n1++;
                    if (n2[0] == '$') n2++;
                    if (strcmp(n1, n2) == 0) {
                        local_idx = scopes_locales[s].indices_textos[v];
                        break;
                    }
                }
                if (local_idx != -1) break;
            }
        }
        if (local_idx >= 0) {
            strncpy(texto_vars[local_idx].valor, entrada, MAX_TEXTO_LEN - 1);
            texto_vars[local_idx].valor[MAX_TEXTO_LEN - 1] = '\0';
            return;
        }

        if (es_numero) agregar_variable(nombre_var, (int)valor_num);
        else agregar_texto_var(nombre_var, entrada);
        return;
    }       
    }
    fprintf(stderr, "Error: Variable '$%s' no declarada.\n", nombre_var);
}

/* PROCESAR LEERCARACTER */
void procesar_leercaracter(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr == '(') {
        ptr++;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
    }
    
    int i = 0;
    if (*ptr == '$') ptr++;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_var[i++] = *ptr++;
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    if (strlen(nombre_var) == 0) {
        fprintf(stderr, "Error: LEERCARACTER requiere una variable válida.\n");
        return;
    }
    
    int c = getchar();
    
    int idx = buscar_variable_caracter(nombre_var);
    if (idx >= 0) { variables_caracter[idx].valor = (char)c; while (getchar() != '\n' && !feof(stdin)); return; }
    
    idx = buscar_variable_caracter_sin_signo(nombre_var);
    if (idx >= 0) { variables_caracter_sin_signo[idx].valor = (unsigned char)c; while (getchar() != '\n' && !feof(stdin)); return; }
    
    if (en_funcion) { agregar_variable_caracter(nombre_var, (char)c); while (getchar() != '\n' && !feof(stdin)); return; }
    
    fprintf(stderr, "Error: Variable CARACTER '$%s' no declarada.\n", nombre_var);
}

/* PROCESAR LEERHASTA */
void procesar_leerhasta(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    if (*ptr == '$') ptr++;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_var[i++] = *ptr++;
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    if (strlen(nombre_var) == 0) {
        fprintf(stderr, "Error: LEERHASTA requiere variable TEXTO destino.\n");
        return;
    }
    
    const char *coma = strchr(argumento, ',');
    if (!coma) {
        fprintf(stderr, "Error: LEERHASTA requiere terminador.\n");
        return;
    }
    
    const char *terminador_ptr = coma + 1;
    while (*terminador_ptr == ' ' || *terminador_ptr == '\t') terminador_ptr++;
    
    char terminador[MAX_TEXTO_LEN] = "";
    i = 0;
    if (*terminador_ptr == '"') {
        terminador_ptr++;
        while (*terminador_ptr && *terminador_ptr != '"' && i < MAX_TEXTO_LEN - 1) {
            terminador[i++] = *terminador_ptr++;
        }
    } else {
        while (*terminador_ptr && *terminador_ptr != ')' && i < MAX_TEXTO_LEN - 1) {
            terminador[i++] = *terminador_ptr++;
        }
    }
    terminador[i] = '\0';
    limpiar_string(terminador);
    
    if (strlen(terminador) == 0) {
        fprintf(stderr, "Error: LEERHASTA requiere terminador válido.\n");
        return;
    }
    
    int idx = buscar_texto_var(nombre_var);
    if (idx < 0) {
        fprintf(stderr, "Error: Texto '$%s' no declarado.\n", nombre_var);
        return;
    }
    
    texto_vars[idx].valor[0] = '\0';
    
    char linea[MAX_LINEA];
    int primera_linea = 1;
    
    fprintf(stderr, "Ingresá texto (ingresá '%s' en una línea sola para terminar):\n", terminador);
    while (fgets(linea, MAX_LINEA, stdin)) {
        size_t len = strlen(linea);
        if (len > 0 && linea[len-1] == '\n') {
            linea[len-1] = '\0';
            len--;
        }
        
        if (strcmp(linea, terminador) == 0) break;
        
        if (!primera_linea) {
            if (strlen(texto_vars[idx].valor) + 1 < MAX_TEXTO_LEN) strcat(texto_vars[idx].valor, "\n");
        }
        
        if (strlen(texto_vars[idx].valor) + len < MAX_TEXTO_LEN) strcat(texto_vars[idx].valor, linea);
        primera_linea = 0;
    }
}

/* FUNCIONES DE COLORES */
int obtener_codigo_color_texto(const char *color) {
    if (strcmp(color, "negro") == 0) return 30;
    if (strcmp(color, "rojo") == 0) return 31;
    if (strcmp(color, "verde") == 0) return 32;
    if (strcmp(color, "amarillo") == 0) return 33;
    if (strcmp(color, "azul") == 0) return 34;
    if (strcmp(color, "magenta") == 0) return 35;
    if (strcmp(color, "cyan") == 0) return 36;
    if (strcmp(color, "blanco") == 0) return 37;
    if (strcmp(color, "gris") == 0) return 90;
    if (strcmp(color, "rojoclaro") == 0) return 91;
    if (strcmp(color, "verdeclaro") == 0) return 92;
    if (strcmp(color, "amarilloclaro") == 0) return 93;
    if (strcmp(color, "azulclaro") == 0) return 94;
    if (strcmp(color, "magentaclaro") == 0) return 95;
    if (strcmp(color, "cyanclaro") == 0) return 96;
    if (strcmp(color, "blancoclaro") == 0) return 97;
    return -1;
}

int obtener_codigo_color_fondo(const char *color) {
    if (strcmp(color, "negro") == 0) return 40;
    if (strcmp(color, "rojo") == 0) return 41;
    if (strcmp(color, "verde") == 0) return 42;
    if (strcmp(color, "amarillo") == 0) return 43;
    if (strcmp(color, "azul") == 0) return 44;
    if (strcmp(color, "magenta") == 0) return 45;
    if (strcmp(color, "cyan") == 0) return 46;
    if (strcmp(color, "blanco") == 0) return 47;
    if (strcmp(color, "gris") == 0) return 100;
    if (strcmp(color, "rojoclaro") == 0) return 101;
    if (strcmp(color, "verdeclaro") == 0) return 102;
    if (strcmp(color, "amarilloclaro") == 0) return 103;
    if (strcmp(color, "azulclaro") == 0) return 104;
    if (strcmp(color, "magentaclaro") == 0) return 105;
    if (strcmp(color, "cyanclaro") == 0) return 106;
    if (strcmp(color, "blancoclaro") == 0) return 107;
    return -1;
}

void procesar_colortexto(const char *argumento) {
    if (!argumento) return;
    const char *ptr = argumento;
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    char color[MAX_NOMBRE] = "";
    int i = 0;
    while (*ptr && *ptr != ')' && *ptr != '"' && i < MAX_NOMBRE - 1) color[i++] = *ptr++;
    color[i] = '\0'; limpiar_string(color);
    int codigo = obtener_codigo_color_texto(color);
    if (codigo >= 0) { printf("\033[%dm", codigo); fflush(stdout); }
    else fprintf(stderr, "Error: Color de texto '%s' no válido.\n", color);
}

void procesar_colorfondo(const char *argumento) {
    if (!argumento) return;
    const char *ptr = argumento;
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    char color[MAX_NOMBRE] = "";
    int i = 0;
    while (*ptr && *ptr != ')' && *ptr != '"' && i < MAX_NOMBRE - 1) color[i++] = *ptr++;
    color[i] = '\0'; limpiar_string(color);
    int codigo = obtener_codigo_color_fondo(color);
    if (codigo >= 0) { printf("\033[%dm", codigo); fflush(stdout); }
    else fprintf(stderr, "Error: Color de fondo '%s' no válido.\n", color);
}

void procesar_resetcolor(const char *argumento) {
    (void)argumento; printf("\033[0m"); fflush(stdout); 
}

void procesar_textonegrita(const char *argumento) {
    (void)argumento; printf("\033[1m"); fflush(stdout); 
}

void procesar_textocursiva(const char *argumento) {
    (void)argumento; printf("\033[3m"); fflush(stdout); 
}

void procesar_textosubrayado(const char *argumento) {
    (void)argumento; printf("\033[4m"); fflush(stdout); 
}

void procesar_textoreset(const char *argumento) {
    (void)argumento; printf("\033[0m"); fflush(stdout); 
}

/* OTRAS FUNCIONES */
void procesar_limpiarpantalla(void) {
#ifdef _WIN32
    (void)system("cls");
#else
    (void)system("clear");
#endif
}

void procesar_esperar(const char *argumento) {
    if (!argumento) return;
    char arg[MAX_LINEA];
    strncpy(arg, argumento, MAX_LINEA - 1);
    arg[MAX_LINEA - 1] = '\0';
    arg[MAX_LINEA - 1] = '\0';
    limpiar_string(arg);
    
    char valor_str[MAX_LINEA] = "";
    int i = 0;
    const char *ptr = arg;
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr >= '0' && *ptr <= '9' && i < MAX_LINEA - 1) valor_str[i++] = *ptr++;
    valor_str[i] = '\0';
    
    int tiempo = atoi(valor_str);
    if (tiempo <= 0) {
        fprintf(stderr, "Error: ESPERAR requiere un valor numérico positivo.\n");
        return;
    }
    
    if (strstr(ptr, "MICROSEGUNDOS") != NULL) usleep(tiempo);
    else if (strstr(ptr, "MILISEGUNDOS") != NULL) usleep(tiempo * 1000);
    else if (strstr(ptr, "SEGUNDOS") != NULL) sleep(tiempo);
    else {
        fprintf(stderr, "Error: ESPERAR requiere unidad de tiempo explícita.\n");
        fprintf(stderr, "Uso válido:\n  ESPERAR(100, MILISEGUNDOS)\n ESPERAR (500000, MICROSEGUNDOS)\n ESPERAR (1, SEGUNDOS).\n");
        return;
    }
}

void procesar_sistema(const char *argumento) {
    if (!argumento) return;
    char cmd[MAX_LINEA];
    strncpy(cmd, argumento, MAX_LINEA - 1);
    cmd[MAX_LINEA - 1] = '\0';
    cmd[MAX_LINEA - 1] = '\0';
    limpiar_string(cmd);
    (void)system(cmd);
}

/* POSICIONAR CURSOR EN CONSOLA - CROSS-PLATFORM */
#ifdef _WIN32
#include <windows.h>
#endif

void nico_posicionar_cursor(int fila, int columna) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;
    COORD coord = { (SHORT)(columna - 1), (SHORT)(fila - 1) };
    SetConsoleCursorPosition(hConsole, coord);
#else
    printf("\033[%d;%dH", fila, columna);
#endif
    fflush(stdout);
}
