/*
 * Nico v1.0 - Intérprete Educativo de Scripting en Español
 * @file:         main.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Punto de entrada, carga de archivos, bucle principal de 
 *                ejecución, gestión de pilas de bloques y modo interactivo.
 */
#include "nico.h"
#include "nico_gpio.h"
#include <unistd.h>
#include <limits.h>
#include <float.h>

/* DECLARACIONES FORWARD */
int modo_estricto = 1;

void procesar_textonegrita(const char *argumento);
void procesar_textocursiva(const char *argumento);
void procesar_textosubrayado(const char *argumento);
void procesar_textoreset(const char *argumento);
void procesar_funcion_texto(const char *linea);

int encontrar_fin_para(int linea_inicio, int nivel);
int encontrar_fin_proceder(int linea_inicio, int nivel);
int encontrar_fin_mientras(int linea_inicio, int nivel_buscado);
int encontrar_fin_subprograma(int linea_inicio);
int encontrar_fin_funcion(int linea_inicio);
int ejecutar_salto_a_etiqueta(const char *nombre, int linea_actual);
int get_var_valor_global(const char *nombre);
void set_var_valor_global(const char *nombre, int valor);

/* GPIO Raspberry Pi */
void procesar_gpio_configurar(const char *argumento);
void procesar_gpio_estado_pin(const char *argumento);
void procesar_gpio_leer(const char *argumento);
void procesar_gpio_pwm(const char *argumento);
void procesar_gpio_detener_pwm(const char *argumento);
void procesar_gpio_rf(const char *argumento);
void procesar_gpio_detener_rf(const char *argumento);

/* VARIABLES GLOBALES */
char lineas_programa[MAX_LINEAS_PROGRAMA][MAX_LINEA];
int num_lineas_programa = 0;

MientrasBloque mientras_stack[MAX_NESTING];
int mientras_stack_ptr = 0;

ProcederBloque proceder_stack[MAX_NESTING];
int proceder_stack_ptr = 0;

SiBloque si_stack[MAX_NESTING];
int si_stack_ptr = 0;

ParaBloque para_stack[MAX_NESTING];
int para_stack_ptr = 0;

SegunBloque segun_stack[MAX_NESTING];
int segun_stack_ptr = 0;

SubBloque sub_stack[MAX_NESTING];
int sub_stack_ptr = 0;

FuncionBloque funcion_stack[MAX_NESTING];
int funcion_stack_ptr = 0;

int en_bloque_principal = 0;
int en_subprograma = 0;
int en_funcion = 0;
int error_fatal = 0;
int fase_declaraciones = 1;
int fin_principal_encontrado = 0;

char funcion_variable_destino[MAX_NOMBRE] = "";
char funcion_destino_stack[MAX_NESTING][MAX_NOMBRE];
int funcion_destino_stack_ptr = 0;

/* FUNCIONES AUXILIARES */
void limpiar_memoria_completa(void) {
    num_variables = 0;
    num_variables_sin_signo = 0;
    num_variables_decimal = 0;
    num_variables_decimal_sin_signo = 0;
    num_variables_caracter = 0;
    num_variables_caracter_sin_signo = 0;
    num_texto_vars = 0;
    num_constantes = 0;
    num_constantes_sin_signo = 0;
    num_constantes_decimal = 0;
    num_constantes_decimal_sin_signo = 0;
    num_constantes_caracter = 0;
    num_constantes_caracter_sin_signo = 0;
    num_texto_constantes = 0;
    num_listas_enteras = 0;
    num_listas_decimales = 0;
    num_listas_enteras_sin_signo = 0;
    num_listas_decimales_sin_signo = 0;
    num_etiquetas = 0;
    cerrar_todos_los_archivos();
    num_variables_archivo = 0;
    num_lineas_programa = 0;
    mientras_stack_ptr = 0;
    proceder_stack_ptr = 0;
    si_stack_ptr = 0;
    para_stack_ptr = 0;
    segun_stack_ptr = 0;
    sub_stack_ptr = 0;
    funcion_stack_ptr = 0;
    fase_constantes = 1;
    fase_variables = 0;
    en_bloque_principal = 0;
    en_subprograma = 0;
    en_funcion = 0;
    fase_declaraciones = 1;
    fin_principal_encontrado = 0;
    num_subprogramas_registrados = 0;
    num_funciones_registradas = 0;
    valor_retorno_funcion = 0;
    hay_valor_retorno = 0;
    funcion_variable_destino[0] = '\0';
    funcion_destino_stack_ptr = 0;
    num_matriz_enteras = 0;
    num_matriz_decimales = 0;
    num_matriz_enteras_sin_signo = 0;
    num_matriz_decimales_sin_signo = 0;
}

void mostrar_prompt(void) {
    printf(">>> ");
    fflush(stdout);
}

void comando_ayuda(void) {
    fprintf(stderr, "\n");
    fprintf(stderr, "╔════════════════════════════════════════════════════════╗\n");
    fprintf(stderr, "║           COMANDOS DEL INTÉRPRETE NICO                 ║\n");
    fprintf(stderr, "╠════════════════════════════════════════════════════════╣\n");
    fprintf(stderr, "║  USAR archivo.nico   Carga un archivo .nico            ║\n");
    fprintf(stderr, "║  CORRER              Ejecuta el programa cargado       ║\n");
    fprintf(stderr, "║  CERRAR              Cierra el programa actual         ║\n");
    fprintf(stderr, "║  RANGOS              Muestra rangos de variables       ║\n");
    fprintf(stderr, "║  SALIR               Sale del intérprete               ║\n");
    fprintf(stderr, "║  ?                   Muestra esta ayuda                ║\n");
    fprintf(stderr, "╚════════════════════════════════════════════════════════╝\n");
    fprintf(stderr, "\n");
}

void comando_rangos(void) {
    fprintf(stderr, "\n");
    fprintf(stderr, "   RANGOS DE VARIABLES - NICO v1.0\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES ENTERAS:\n");
    fprintf(stderr, "     Rango: %d a %d\n", INT_MIN, INT_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES ENTERAS SIN SIGNO:\n");
    fprintf(stderr, "     Rango: 0 a %u\n", UINT_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_SIN_SIGNO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES DECIMALES:\n");
    fprintf(stderr, "     Rango: %e a %e\n", -DBL_MAX, DBL_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_DECIMAL);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES DECIMALES SIN SIGNO:\n");
    fprintf(stderr, "     Rango: 0.0 a %e\n", DBL_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_DECIMAL_SIN_SIGNO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES CARACTER:\n");
    fprintf(stderr, "     Rango: %d a %d (ASCII)\n", SCHAR_MIN, SCHAR_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_CARACTER);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES CARACTER SIN SIGNO:\n");
    fprintf(stderr, "     Rango: 0 a %u (ASCII)\n", UCHAR_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_CARACTER_SIN_SIGNO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES TEXTO:\n");
    fprintf(stderr, "     Máximo: %d caracteres por variable\n", MAX_TEXTO_LEN - 1);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_TEXTO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   CONSTANTES (todos los tipos):\n");
    fprintf(stderr, "     Máximo: %d constantes\n", MAX_CONSTANTES);
    fprintf(stderr, "     Máximo: %d constantes sin signo\n", MAX_CONSTANTES_SIN_SIGNO);
    fprintf(stderr, "     Máximo: %d constantes decimales\n", MAX_CONSTANTES_DECIMAL);
    fprintf(stderr, "     Máximo: %d constantes texto\n", MAX_CONSTANTES_TEXTO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   LISTAS / ARRAYS:\n");
    fprintf(stderr, "     Máximo: %d elementos por lista\n", MAX_LISTA);
    fprintf(stderr, "     Máximo: %d listas enteras\n", MAX_LISTAS_ENTERAS);
    fprintf(stderr, "     Máximo: %d listas decimales\n", MAX_LISTAS_DECIMALES);
    fprintf(stderr, "     Máximo: %d listas sin signo\n", MAX_LISTAS_ENTERAS_SIN_SIGNO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   ARCHIVOS:\n");
    fprintf(stderr, "     Máximo: %d archivos simultáneos\n", MAX_VARS_ARCHIVO);
    fprintf(stderr, "     Modos: 0=Escritura, 1=Append, 2=Lectura, 3=L+E\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   FUNCIONES:\n");
    fprintf(stderr, "     Máximo: %d niveles de anidamiento\n", MAX_NESTING);
    fprintf(stderr, "     Máximo: %d parámetros por función\n", MAX_PARAMETROS);
    fprintf(stderr, "\n");
    fprintf(stderr, "   PROGRAMA:\n");
    fprintf(stderr, "     Máximo: %d líneas de código\n", MAX_LINEAS_PROGRAMA);
    fprintf(stderr, "     Máximo: %d caracteres por línea\n", MAX_LINEA - 1);
    fprintf(stderr, "\n");
}

int validar_estructura_programa(char *nombre_programa) {
    int inicio_encontrado = 0, fin_encontrado = 0;
    int bloque_principal_encontrado = 0;
    int fin_principal_encontrado_local = 0;
    
    for (int i = 0; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (!inicio_encontrado) {
            if (comienza_con(linea, "PROGRAMA")) {
                char *ptr = linea + 8;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                sscanf(ptr, "%s", nombre_programa);
                if (!strlen(nombre_programa)) {
                    fprintf(stderr, "Error línea %d: PROGRAMA requiere un nombre.\n", i + 1);
                    return 0;
                }
                inicio_encontrado = 1;
            } else {
                fprintf(stderr, "Error línea %d: El programa debe comenzar con PROGRAMA.\n", i + 1);
                return 0;
            }
            continue;
        }
        
        if (comienza_con(linea, "BLOQUE PRINCIPAL")) {
        scope_actual = -1;  
        bloque_principal_encontrado = 1;
        }

        if (comienza_con(linea, "FIN PRINCIPAL")) {
            fin_principal_encontrado_local = 1;
        }
        if (comienza_con(linea, "FINAL")) {
            fin_encontrado = 1;
            break;
        }
    }
    
    if (!inicio_encontrado) {
        fprintf(stderr, "Error: No se encontro sentencia PROGRAMA.\n");
        return 0;
    }
    if (!bloque_principal_encontrado) {
        fprintf(stderr, "Error: El programa no tiene BLOQUE PRINCIPAL.\n");
        return 0;
    }
    if (!fin_principal_encontrado_local) {
        fprintf(stderr, "Error: El programa no tiene FIN PRINCIPAL.\n");
        return 0;
    }
    if (!fin_encontrado) {
        fprintf(stderr, "Error: El programa no tiene sentencia FINAL.\n");
        return 0;
    }
    return 1;
}

int cargar_archivo_en_memoria(const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "r");
    if (!archivo) return -1;
    
    num_lineas_programa = 0;
    char buffer[MAX_LINEA];
    while (fgets(buffer, MAX_LINEA, archivo) && num_lineas_programa < MAX_LINEAS_PROGRAMA) {
        limpiar_string(buffer);
        strncpy(lineas_programa[num_lineas_programa], buffer, MAX_LINEA - 1);
        lineas_programa[num_lineas_programa][MAX_LINEA - 1] = '\0';
        num_lineas_programa++;
    }
    fclose(archivo);
    return 0;
}

/* EJECUTAR LLAMADA A FUNCION */
int ejecutar_llamada_funcion(const char *linea, int linea_actual, int linea_num) {
    char *igual = strchr(linea, '=');
    if (!igual) return -1;
    
    char lado_izq[MAX_LINEA];
    int len_izq = igual - linea;
    if (len_izq >= MAX_LINEA) len_izq = MAX_LINEA - 1;
    strncpy(lado_izq, linea, len_izq);
    lado_izq[len_izq] = '\0';
    limpiar_string(lado_izq);
    
    char nombre_variable[MAX_NOMBRE] = "";
    char *ptr = lado_izq;
    if (comienza_con(ptr, "CALCULAR EN")) {
        ptr += 11;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
    }
    if (*ptr == '$') ptr++;
    int i = 0;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_variable[i++] = *ptr++;
    nombre_variable[i] = '\0';
    
    char *fin_igual = igual + 1;
    while (*fin_igual == ' ' || *fin_igual == '\t') fin_igual++;
    
    char nombre_funcion[MAX_NOMBRE];
    i = 0;
    ptr = fin_igual;
    while (*ptr && *ptr != '(' && !isspace((unsigned char)*ptr) && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_funcion[i++] = *ptr++;
    }
    nombre_funcion[i] = '\0';
    limpiar_string(nombre_funcion);
    
    int func_idx = buscar_funcion_info(nombre_funcion);
    if (func_idx < 0) {
        fprintf(stderr, "Error línea %d: Función '$%s' no declarada.\n", linea_actual, nombre_funcion);
        return -1;
    }
    
    char *args[MAX_PARAMETROS];
    int num_args = 0;
    if (*ptr == '(') {
        ptr++;
        while (*ptr && *ptr != ')' && num_args < MAX_PARAMETROS) {
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            char arg[MAX_LINEA] = "";
            i = 0;
            int nivel_parentesis = 0;
            while (*ptr && (nivel_parentesis > 0 || (*ptr != ',' && *ptr != ')')) && i < MAX_LINEA - 1) {
                if (*ptr == '(') nivel_parentesis++;
                else if (*ptr == ')') nivel_parentesis--;
                arg[i++] = *ptr++;
            }
            arg[i] = '\0';
            limpiar_string(arg);
            if (strlen(arg) > 0) args[num_args++] = strdup(arg);
            if (*ptr == ',') ptr++;
        }
    }
    
    if (num_args != funciones_registradas[func_idx].num_params) {
        fprintf(stderr, "Error línea %d: Función '$%s' espera %d parámetros, se recibieron %d.\n",
                linea_actual, nombre_funcion,
                funciones_registradas[func_idx].num_params, num_args);
        for (int k = 0; k < num_args; k++) free(args[k]);
        return -1;
    }
    
    if (funcion_destino_stack_ptr < MAX_NESTING) {
        strncpy(funcion_destino_stack[funcion_destino_stack_ptr], nombre_variable, MAX_NOMBRE - 1);
        funcion_destino_stack[funcion_destino_stack_ptr][MAX_NOMBRE - 1] = '\0';
        funcion_destino_stack_ptr++;
    }
    strncpy(funcion_variable_destino, nombre_variable, MAX_NOMBRE - 1);
    funcion_variable_destino[MAX_NOMBRE - 1] = '\0';
    
    int exito;
    double resultado = llamar_funcion(nombre_funcion, args, num_args, &exito);
    
    for (int k = 0; k < num_args; k++) free(args[k]);
    
    if (!exito) {
        if (funcion_destino_stack_ptr > 0) funcion_destino_stack_ptr--;
        if (funcion_destino_stack_ptr >= 0) {
            strncpy(funcion_variable_destino, funcion_destino_stack[funcion_destino_stack_ptr], MAX_NOMBRE - 1);
            funcion_variable_destino[MAX_NOMBRE - 1] = '\0';
        } else {
            funcion_variable_destino[0] = '\0';
        }
        return -1;
    }
    
    if (nombre_variable[0] != '\0') {
        char *nombre = nombre_variable;
        if (nombre[0] == '$') nombre++;
        
        int idx = buscar_variable(nombre);
        if (idx >= 0) { variables[idx].valor = (int)resultado; }
        else {
            idx = buscar_variable_sin_signo(nombre);
            if (idx >= 0) { variables_sin_signo[idx].valor = (unsigned int)resultado; }
            else {
                idx = buscar_variable_decimal(nombre);
                if (idx >= 0) { variables_decimal[idx].valor = resultado; }
                else {
                    idx = buscar_variable_decimal_sin_signo(nombre);
                    if (idx >= 0) { variables_decimal_sin_signo[idx].valor = resultado; }
                    else {
                        idx = buscar_variable_caracter(nombre);
                        if (idx >= 0) { variables_caracter[idx].valor = (char)resultado; }
                        else {
                            idx = buscar_variable_caracter_sin_signo(nombre);
                            if (idx >= 0) { variables_caracter_sin_signo[idx].valor = (unsigned char)resultado; }
                            else {
                                agregar_variable_decimal(nombre, resultado);
                            }
                        }
                    }
                }
            }
        }
    }

    if (funcion_destino_stack_ptr > 0) {
        funcion_destino_stack_ptr--;
        if (funcion_destino_stack_ptr >= 0) {
            strncpy(funcion_variable_destino, funcion_destino_stack[funcion_destino_stack_ptr], MAX_NOMBRE - 1);
            funcion_variable_destino[MAX_NOMBRE - 1] = '\0';
        } else {
            funcion_variable_destino[0] = '\0';
        }
    }
    
    return linea_num + 1;
}

/* ASIGNAR VALOR DE RETORNO DE FUNCION */
void asignar_valor_retorno_funcion(void) {
    if (funcion_variable_destino[0] == '\0') return;
    if (!hay_valor_retorno) return;
    
    char *nombre = funcion_variable_destino;
    if (nombre[0] == '$') nombre++;
    
    int idx = buscar_variable(nombre);
    if (idx >= 0) {
        variables[idx].valor = (int)valor_retorno_funcion;
        return;
    }
    
    idx = buscar_variable_sin_signo(nombre);
    if (idx >= 0) {
        variables_sin_signo[idx].valor = (unsigned int)valor_retorno_funcion;
        return;
    }
    
    idx = buscar_variable_decimal(nombre);
    if (idx >= 0) {
        variables_decimal[idx].valor = valor_retorno_funcion;
        return;
    }
    
    idx = buscar_variable_decimal_sin_signo(nombre);
    if (idx >= 0) {
        variables_decimal_sin_signo[idx].valor = valor_retorno_funcion;
        return;
    }
    
    idx = buscar_variable_caracter(nombre);
    if (idx >= 0) {
        variables_caracter[idx].valor = (char)valor_retorno_funcion;
        return;
    }
    
    idx = buscar_variable_caracter_sin_signo(nombre);
    if (idx >= 0) {
        variables_caracter_sin_signo[idx].valor = (unsigned char)valor_retorno_funcion;
        return;
    }
    
    agregar_variable_decimal(nombre, valor_retorno_funcion);
}

/* EJECUTAR PROGRAMA CARGADO */
int ejecutar_programa_cargado(void) {
    inicializar_semilla_aleatoria();
    scope_actual = -1;
    for (int i = 0; i < MAX_SCOPES; i++) {
        scopes_locales[i].num_variables = 0;
    }
    funcion_stack_ptr = 0;
    if (num_lineas_programa == 0) {
        fprintf(stderr, "Error: No hay ningún programa cargado.\n");
        return -1;
    }
    
    funcion_destino_stack_ptr = 0;
    funcion_variable_destino[0] = '\0';
    hay_valor_retorno = 0;
    valor_retorno_funcion = 0;
    fin_principal_encontrado = 0;
    
    char nombre_programa[MAX_NOMBRE] = "";
    int declaraciones_permitidas = 1;
    
    if (!validar_estructura_programa(nombre_programa)) return -1;
    
    fprintf(stderr, "> Programa: %s\n", nombre_programa);
    
    registrar_todas_las_etiquetas();
    registrar_todos_los_subprogramas();
    
    for (int i = 0; i < num_subprogramas_registrados; i++) {
        int inicio_sub = subprogramas_registrados[i].linea_inicio + 1;
        int fin_sub = encontrar_fin_subprograma(subprogramas_registrados[i].linea_inicio);
        if (fin_sub != -1 && inicio_sub <= fin_sub) {
            validar_estructura_bloques(inicio_sub, fin_sub);
        }
    }

    if (registrar_todas_las_funciones() != 0) {
        fprintf(stderr, "Error: Hay funciones inválidas. Ejecución finalizada.\n");
        return -1;
    }    

    fprintf(stderr, "> Validando nombres de variables y parámetros...\n");
    
    for (int f = 0; f < num_funciones_registradas; f++) {
        for (int p = 0; p < funciones_registradas[f].num_params; p++) {
            char *param = funciones_registradas[f].params[p];
            
            if (buscar_variable(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con otra variable.\n", funciones_registradas[f].nombre, param);
            }
            else if (buscar_variable_sin_signo(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con variable global ENTERA SIN SIGNO.\n", funciones_registradas[f].nombre, param);
            }
            else if (buscar_variable_decimal(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con variable global DECIMAL.\n", funciones_registradas[f].nombre, param);
            }
            else if (buscar_variable_decimal_sin_signo(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con variable global DECIMAL SIN SIGNO.\n", funciones_registradas[f].nombre, param);
            }
            else if (buscar_variable_caracter(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con variable global CARACTER.\n", funciones_registradas[f].nombre, param);
            }
            else if (buscar_variable_caracter_sin_signo(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con variable global CARACTER SIN SIGNO.\n", funciones_registradas[f].nombre, param);
            }
        }
    }
    
    int linea_num = 0;
    int inicio_encontrado = 0;
    
    while (linea_num < num_lineas_programa) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[linea_num], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        saltar_espacios_inplace(linea);
        remover_comentario(linea);
        
        int linea_actual = linea_num + 1;
        
        if (!strlen(linea)) {
            linea_num++;
            continue;
        }
        
        if (!inicio_encontrado) {
            if (comienza_con(linea, "PROGRAMA")) {
                inicio_encontrado = 1;
            } else {
                fprintf(stderr, "Error línea %d.\n", linea_actual);
                return -1;
            }
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "FINAL")) {
            if (!fin_principal_encontrado) {
                fprintf(stderr, "Error: El programa no tiene FIN PRINCIPAL antes de FINAL.\n");
                return -1;
            }
            nico_gpio_cleanup();
            fprintf(stderr, "> Programa '%s' finalizado.\n", nombre_programa);
            cerrar_todos_los_archivos();
            return 0;
        }
        
        if (comienza_con(linea, "BLOQUE PRINCIPAL")) {
            if (en_bloque_principal) {
                fprintf(stderr, "Error línea %d: Ya hay un BLOQUE PRINCIPAL abierto.\n", linea_actual);
                return -1;
            }
            en_bloque_principal = 1;
            fase_declaraciones = 0;
        
            {
                int inicio_main = linea_num + 1;
                int fin_main = -1;

                for (int v = inicio_main; v < num_lineas_programa; v++) {
                    char lb[MAX_LINEA];
                    strncpy(lb, lineas_programa[v], MAX_LINEA-1);
                    lb[MAX_LINEA-1] = '\0';
                    limpiar_string(lb);
                    remover_comentario(lb);
                    if (comienza_con(lb, "FIN PRINCIPAL")) {
                        fin_main = v;
                        break;
                    }
                }

                if (fin_main != -1) {
                    validar_estructura_bloques(inicio_main, fin_main);
                }
            }

            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "FIN PRINCIPAL")) {
            if (!en_bloque_principal) {
                fprintf(stderr, "Error línea %d: FIN PRINCIPAL sin BLOQUE PRINCIPAL.\n", linea_actual);
                return -1;
            }
            en_bloque_principal = 0;
            fin_principal_encontrado = 1;
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "FIN SI")) {
            if (si_stack_ptr > 0) si_stack_ptr--;
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "FIN SEGUN")) {
            if (segun_stack_ptr > 0) segun_stack_ptr--;
            linea_num++;
            continue;
        }
        
        if (strncmp(linea, "FIN PARA", 8) == 0) {
            if (para_stack_ptr > 0) {
                ParaBloque *bloque = &para_stack[para_stack_ptr - 1];
                int valor = get_var_valor_global(bloque->var_nombre);
                valor += bloque->paso;
                set_var_valor_global(bloque->var_nombre, valor);
                int continuar = 0;
                if (bloque->paso > 0) {
                    if (valor <= bloque->fin) continuar = 1;
                } else {
                    if (valor >= bloque->fin) continuar = 1;
                }
                if (continuar) {
                    linea_num = bloque->linea_inicio + 1;
                } else {
                    para_stack_ptr--;
                    linea_num++;
                }
            } else {
                fprintf(stderr, "Error línea %d: FIN PARA sin PARA.\n", linea_actual);
                linea_num++;
            }
            continue;
        }
       
        if (strncmp(linea, "FIN MIENTRAS", 12) == 0) {
            int nivel = 0;
            int linea_mientras = linea_num - 1;
            int while_line = -1;
    
            while (linea_mientras >= 0) {
                char linea_prev[MAX_LINEA];
                strncpy(linea_prev, lineas_programa[linea_mientras], MAX_LINEA - 1);
                linea_prev[MAX_LINEA - 1] = '\0';
                limpiar_string(linea_prev);
                remover_comentario(linea_prev);
        
                if (strncmp(linea_prev, "FIN MIENTRAS", 12) == 0) {
                    nivel++;
                }
                else if (strncmp(linea_prev, "MIENTRAS", 8) == 0 && strstr(linea_prev, "HACER")) {
                    if (nivel == 0) {
                        while_line = linea_mientras;
                        break;
                    }
                    nivel--;
                }
                linea_mientras--;
            }
            if (while_line != -1) {
                int exito = 0;
                int resultado = 0;
                char condicion[MAX_LINEA] = "";
                const char *src = lineas_programa[while_line];
                
                condicion[0] = '\0';
                const char *p_abre = strchr(src, '(');
                const char *p_cierra = p_abre ? strchr(p_abre + 1, ')') : NULL;
                
                if (p_abre && p_cierra && p_cierra > p_abre) {
                    int len = (int)(p_cierra - p_abre - 1);
                    if (len > 0 && len < MAX_LINEA - 1) {
                        memcpy(condicion, p_abre + 1, len);
                        condicion[len] = '\0';
                    }
                }
                
                char *inicio = condicion;
                while (*inicio == ' ' || *inicio == '\t') inicio++;
                char *fin = inicio + strlen(inicio) - 1;
                while (fin > inicio && (*fin == ' ' || *fin == '\t' || *fin == '\r')) {
                    *fin = '\0';
                    fin--;
                }
                if (inicio != condicion) {
                    memmove(condicion, inicio, strlen(inicio) + 1);
                }
                
                char cond_eval[MAX_LINEA + 2];
                cond_eval[0] = ' ';
                strcpy(cond_eval + 1, condicion);
                
                resultado = evaluar_condicion(cond_eval, &exito);
            
                if (exito && resultado) {
                    linea_num = while_line + 1;
                } else {
                    if (mientras_stack_ptr > 0) {
                        mientras_stack_ptr--;
                    }
                    linea_num++;
                }
            } else {
                if (mientras_stack_ptr > 0) {
                    mientras_stack_ptr--;
                }
                linea_num++;
            }
            continue;    
        }   
        
        if (comienza_con(linea, "FIN SUBPROGRAMA")) {
            if (sub_stack_ptr > 0) {
                sub_stack_ptr--;
                linea_num = sub_stack[sub_stack_ptr].linea_retorno;
                en_subprograma = 0;
            } else {
                fprintf(stderr, "Error línea %d: FIN SUBPROGRAMA sin LLAMAR.\n", linea_actual);
                linea_num++;
            }
            continue;
        }
        if (comienza_con(linea, "FIN FUNCION")) {
            if (si_stack_ptr != 0) {
                fprintf(stderr, "Error: Hay %d bloque(s) 'SI' sin cerrar al llegar a FIN FUNCION (línea %d).\n", si_stack_ptr, linea_actual);
                if (modo_estricto) exit(1);
                si_stack_ptr = 0;
            }

            if (funcion_stack_ptr > 0) {
                asignar_valor_retorno_funcion();
                funcion_stack_ptr--;
                linea_num = funcion_stack[funcion_stack_ptr].linea_retorno;
                en_funcion = 0;
                
                if (scope_actual >= 0) {
                    eliminar_scope_local();
                }
                
                funcion_destino_stack_ptr--;
                if (funcion_destino_stack_ptr >= 0) {
                    strncpy(funcion_variable_destino, funcion_destino_stack[funcion_destino_stack_ptr], MAX_NOMBRE - 1);
                    funcion_variable_destino[MAX_NOMBRE - 1] = '\0';
                } else {
                    funcion_variable_destino[0] = '\0';
                }
                linea_num++;
            } else {
                fprintf(stderr, "Error línea %d: FIN FUNCION sin llamada\n", linea_actual);
                linea_num++;
            }
            continue;
        }   
  
        if (comienza_con(linea, "SUBPROGRAMA")) {
            if (fase_declaraciones && !en_bloque_principal) {
                int fin = encontrar_fin_subprograma(linea_num);
                if (fin != -1) {
                    linea_num = fin + 1;
                } else {
                    fprintf(stderr, "Error línea %d: SUBPROGRAMA sin FIN SUBPROGRAMA.\n", linea_actual);
                    return -1;
                }
                continue;
            }
    
            if (en_bloque_principal) {
                int fin = encontrar_fin_subprograma(linea_num);
                if (fin != -1) {
                    linea_num = fin + 1;
                }
                continue;
            }
    
            linea_num++;
            continue;
        }

        if (comienza_con(linea, "FUNCION")) {
            if (fase_declaraciones && !en_bloque_principal) {
                int fin = encontrar_fin_funcion(linea_num);
                if (fin != -1)
                    linea_num = fin + 1;
                else {
                    fprintf(stderr, "Error línea %d: FUNCION sin FIN FUNCION.\n", linea_actual);
                    linea_num++;
                }
                continue;
            }
        }
        
        if (comienza_con(linea, "SALTAR A")) {
            const char *ptr = linea + 8;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            char nombre_etiqueta[MAX_NOMBRE];
            int j = 0;
            while (*ptr && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1)
                nombre_etiqueta[j++] = *ptr++;
            nombre_etiqueta[j] = '\0';
            int linea_destino = ejecutar_salto_a_etiqueta(nombre_etiqueta, linea_actual);
            if (linea_destino == -1) {
                linea_num++;
                continue;
            }
            linea_num = linea_destino;
            continue;
        }
        
        if (comienza_con(linea, "RETORNAR")) {
            if (!en_funcion) {
                fprintf(stderr, "Error línea %d: RETORNAR solo permitido dentro de FUNCION.\n", linea_actual);
                return -1;
            }
            const char *ptr = linea + 8;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            if (ejecutar_retornar(ptr, linea_actual) < 0) {
                linea_num++;
                continue;
            }
            if (funcion_stack_ptr > 0) {
                int tipo = funcion_stack[funcion_stack_ptr - 1].tipo_retorno;
                linea_num = funciones_registradas[tipo].linea_fin;
            }
            continue;
        }
        
        if (*linea == '$' && strchr(linea, '=') != NULL) {
            if (!comienza_con(linea, "CALCULAR EN") && 
                !comienza_con(linea, "RESULTADO EN") && 
                !comienza_con(linea, "ASIGNAR EN")) {
                
                fprintf(stderr, "Error línea %d: Use CALCULAR EN, RESULTADO EN o ASIGNAR EN para asignaciones.\n", linea_actual);
                fprintf(stderr, "Ejemplo válido: CALCULAR EN $suma = $var1 + $var2.\n");
                return -1;
            }
        }

        if (comienza_con(linea, "CALCULAR EN")) {
            procesar_calcular(linea + 11);
            if (error_fatal) {
                fprintf(stderr, "Error fatal: Ejecución terminada.\n");
                return -1;
            }
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LLAMAR A")) {
            if (!en_bloque_principal && !en_subprograma && !en_funcion) {
                fprintf(stderr, "Error línea %d: LLAMAR A solo permitido dentro de BLOQUE PRINCIPAL, SUBPROGRAMA o FUNCION.\n", linea_actual);
                return -1;
            }
            const char *ptr = linea + 8;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            
            char nombre[MAX_NOMBRE];
            int j = 0;
            while (*ptr && *ptr != '(' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1)
                nombre[j++] = *ptr++;
            nombre[j] = '\0';
            if (strlen(nombre) == 0) {
                fprintf(stderr, "Error línea %d: LLAMAR A requiere nombre.\n", linea_actual);
                linea_num++;
                continue;
            }
            
            if (*ptr == '(') {
                const char *inicio = ptr;
                int nivel = 1; ptr++;
                while (*ptr && nivel > 0) {
                    if (*ptr == '(') nivel++;
                    else if (*ptr == ')') nivel--;
                    ptr++;
                }
                
                char expr[MAX_LINEA] = "";
                strncpy(expr, nombre, MAX_NOMBRE - 1);
                expr[MAX_NOMBRE - 1] = '\0';
                
                int len = (int)(ptr - inicio);
                if (len > 0 && len < MAX_LINEA) {
                    strncat(expr, inicio, len);
                }
                
                int exito = 0;
                evaluar_expresion_completa(expr, &exito);
                if (!exito) {
                    fprintf(stderr, "Error línea %d: No se pudo ejecutar función '$%s'.\n", linea_actual, nombre);
                }
                linea_num++;
                continue;
                
            } else {
                char *args[MAX_PARAMETROS];
                int num_args = 0;
                if (*ptr == '(') {
                    ptr++;
                    while (*ptr && *ptr != ')' && num_args < MAX_PARAMETROS) {
                        while (*ptr == ' ' || *ptr == '\t') ptr++;
                        char arg[MAX_LINEA] = "";
                        j = 0;
                        int nivel_parentesis = 0;
                        while (*ptr && (nivel_parentesis > 0 || (*ptr != ',' && *ptr != ')')) && j < MAX_LINEA - 1) {
                            if (*ptr == '(') nivel_parentesis++;
                            else if (*ptr == ')') nivel_parentesis--;
                            arg[j++] = *ptr++;
                        }
                        arg[j] = '\0';
                        limpiar_string(arg);
                        if (strlen(arg) > 0) {
                            args[num_args++] = strdup(arg);
                        }
                        if (*ptr == ',') ptr++;
                    }
                }
                
                int sub_idx = buscar_subprograma_info(nombre);
                
                if (sub_idx == -1) {
                    fprintf(stderr, "Error línea %d: Subprograma '$%s' no declarado.\n", linea_actual, nombre);
                    for (int k = 0; k < num_args; k++) free(args[k]);
                    linea_num++;
                    continue;
                }
                
                if (num_args != subprogramas_registrados[sub_idx].num_params) {
                    fprintf(stderr, "Error línea %d: Subprograma '$%s' espera %d parametros, se recibieron %d.\n",
                            linea_actual, nombre,
                            subprogramas_registrados[sub_idx].num_params, num_args);
                    for (int k = 0; k < num_args; k++) free(args[k]);
                    linea_num++;
                    continue;
                }
                
                if (sub_stack_ptr >= MAX_NESTING) {
                    fprintf(stderr, "Error línea %d: Anidamiento de subprogramas excedido.\n", linea_actual);
                    for (int k = 0; k < num_args; k++) free(args[k]);
                    linea_num++;
                    continue;
                }
                
                asignar_argumentos_a_parametros(
                    subprogramas_registrados[sub_idx].params,
                    subprogramas_registrados[sub_idx].num_params,
                    args,
                    num_args
                );
                
                for (int k = 0; k < num_args; k++) free(args[k]);
                sub_stack[sub_stack_ptr].linea_retorno = linea_num + 1;
                sub_stack_ptr++;
                en_subprograma = 1;
                linea_num = subprogramas_registrados[sub_idx].linea_inicio + 1;
                continue;
            }
        }
        

        if (comienza_con(linea, "RESULTADO EN")) {
            procesar_calcular(linea + 12);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "ASIGNAR EN")) {
            procesar_calcular(linea + 10);
            linea_num++;
            continue;
        }
        
        if (en_subprograma && (comienza_con(linea, "VARIABLE") || comienza_con(linea, "DECLARAR"))) {
            fprintf(stderr, "Error línea %d: Los SUBPROGRAMAS no admiten declarar variables locales.\n", linea_actual);
            fprintf(stderr, "Use variables globales declaradas antes del BLOQUE PRINCIPAL.\n");
            return -1;
        }
        
        if (fase_declaraciones && !en_bloque_principal && !en_subprograma && !en_funcion) {
            if (comienza_con(linea, "VARIABLE ARCHIVO") || comienza_con(linea, "DECLARAR VARIABLE ARCHIVO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_variable_archivo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "CONSTANTE ENTERA SIN SIGNO") || comienza_con(linea, "DECLARAR CONSTANTE ENTERA SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_constante_entera_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }
            
            if (comienza_con(linea, "CONSTANTE DECIMAL SIN SIGNO") || comienza_con(linea, "DECLARAR CONSTANTE DECIMAL SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_constante_decimal_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "CONSTANTE TEXTO") || comienza_con(linea, "DECLARAR CONSTANTE TEXTO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_constante_texto(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "VARIABLE ENTERA SIN SIGNO") || comienza_con(linea, "DECLARAR VARIABLE ENTERA SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_variable_entera_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "VARIABLE DECIMAL SIN SIGNO") || comienza_con(linea, "DECLARAR VARIABLE DECIMAL SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_variable_decimal_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "VARIABLE CARACTER SIN SIGNO") || comienza_con(linea, "DECLARAR VARIABLE CARACTER SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_variable_caracter_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "VARIABLE CARACTER") || comienza_con(linea, "DECLARAR VARIABLE CARACTER")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_variable_caracter(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "LISTA ENTERA SIN SIGNO") || comienza_con(linea, "DECLARAR LISTA ENTERA SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_lista_entera_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "LISTA DECIMAL SIN SIGNO") || comienza_con(linea, "DECLARAR LISTA DECIMAL SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_lista_decimal_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "LISTA ENTERA") || comienza_con(linea, "DECLARAR LISTA ENTERA")) {
                 if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_lista_entera(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "LISTA DECIMAL") || comienza_con(linea, "DECLARAR LISTA DECIMAL")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_lista_decimal(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "LISTA CARACTER SIN SIGNO") || comienza_con(linea, "DECLARAR LISTA CARACTER SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_lista_caracter_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "LISTA CARACTER") || comienza_con(linea, "DECLARAR LISTA CARACTER")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_lista_caracter(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "MATRIZ ENTERA SIN SIGNO") || comienza_con(linea, "DECLARAR MATRIZ ENTERA SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_matriz_entera_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "MATRIZ DECIMAL SIN SIGNO") || comienza_con(linea, "DECLARAR MATRIZ DECIMAL SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_matriz_decimal_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "MATRIZ ENTERA") || comienza_con(linea, "DECLARAR MATRIZ ENTERA")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_matriz_entera(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "MATRIZ DECIMAL") || comienza_con(linea, "DECLARAR MATRIZ DECIMAL")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_matriz_decimal(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "MATRIZ CARACTER SIN SIGNO") || comienza_con(linea, "DECLARAR MATRIZ CARACTER SIN SIGNO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_matriz_caracter_sin_signo(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "MATRIZ CARACTER") || comienza_con(linea, "DECLARAR MATRIZ CARACTER")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_matriz_caracter(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "CONSTANTE ENTERA") || comienza_con(linea, "DECLARAR CONSTANTE ENTERA")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_constante_entera(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "CONSTANTE DECIMAL") || comienza_con(linea, "DECLARAR CONSTANTE DECIMAL")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_constante_decimal(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "VARIABLE ENTERA") || comienza_con(linea, "DECLARAR VARIABLE ENTERA")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_variable_entera(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "VARIABLE DECIMAL") || comienza_con(linea, "DECLARAR VARIABLE DECIMAL")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_variable_decimal(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (comienza_con(linea, "VARIABLE TEXTO") || comienza_con(linea, "DECLARAR VARIABLE TEXTO")) {
                if (!declaraciones_permitidas) {
                    fprintf(stderr, "Error línea %d.\n", linea_actual);
                    return -1;
                }
                if (procesar_declaracion_variable_texto(linea, linea_actual) < 0) return -1;
                linea_num++;
                continue;
            }

            if (!comienza_con(linea, "CONSTANTE") && !comienza_con(linea, "DECLARAR") &&
                !comienza_con(linea, "VARIABLE") && !comienza_con(linea, "LISTA") &&
                !comienza_con(linea, "ETIQUETA") && !comienza_con(linea, "SUBPROGRAMA") &&
                !comienza_con(linea, "FUNCION") && !comienza_con(linea, "VARIABLE ARCHIVO") &&
                !comienza_con(linea, "BLOQUE PRINCIPAL")) {
                fase_declaraciones = 0;
            }
        }
        
        if (!en_bloque_principal && !en_subprograma && !en_funcion) {
            if (comienza_con(linea, "ESCRIBIR") || comienza_con(linea, "MOSTRAR") || comienza_con(linea, "LEER") ||
                comienza_con(linea, "LIMPIARPANTALLA") || comienza_con(linea, "ESPERAR") ||
                comienza_con(linea, "SISTEMA") || comienza_con(linea, "CALCULAR") ||
                comienza_con(linea, "ASIGNAR") || comienza_con(linea, "RESULTADO") ||
                comienza_con(linea, "PARA") || comienza_con(linea, "MIENTRAS") ||
                comienza_con(linea, "REALIZAR") || comienza_con(linea, "SI ") ||
                comienza_con(linea, "RETORNAR")) {
                fprintf(stderr, "Error línea %d: Código ejecutable fuera de BLOQUE PRINCIPAL, SUBPROGRAMA o FUNCION.\n", linea_actual);
                return -1;
            }
        }

        if (strncmp(linea, "PARA", 4) == 0 && strstr(linea, "HACER")) {
            char *p = linea + 4;
            while(*p == ' ') p++;
            int nivel = 0;
            if (*p == '{') {
                p++;
                nivel = atoi(p);
                while(*p && *p != '}') p++;
                if (*p == '}') p++;
            }
            while(*p == ' ') p++;
            if (*p != '(') {
                fprintf(stderr, "Error línea %d: PARA requiere paréntesis: PARA ($var = 0 HASTA 10) HACER.\n", linea_actual);
                int fp = encontrar_fin_para(linea_num, nivel);
                if (fp != -1) linea_num = fp + 1;
                else linea_num++;
                continue;
            }
            p++;
            while(*p == ' ') p++;
            if (*p != '$') {
                fprintf(stderr, "Error línea %d: PARA requiere variable $var.\n", linea_actual);
                int fp = encontrar_fin_para(linea_num, nivel);
                if (fp != -1) linea_num = fp + 1;
                else linea_num++;
                continue;
            }
            p++;
            char var_nombre[MAX_NOMBRE] = "";
            int j = 0;
            while (es_alnum(*p) && j < MAX_NOMBRE-1)
                var_nombre[j++] = *p++;
            var_nombre[j] = '\0';
            while(*p == ' ') p++;
            if (*p == '=') {
                p++;
                while(*p == ' ') p++;
                int valor_inicial = 0;
                if (*p == '$') {
                    p++;
                    char var_ini[MAX_NOMBRE];
                    j = 0;
                    while (es_alnum(*p) && j < MAX_NOMBRE-1)
                        var_ini[j++] = *p++;
                    var_ini[j] = '\0';
                    valor_inicial = get_var_valor_global(var_ini);
                } else {
                    valor_inicial = atoi(p);
                    while (*p && (isdigit(*p) || *p == '-')) p++;
                }
                set_var_valor_global(var_nombre, valor_inicial);
            }
            char *hastaptr = strstr(linea, "HASTA");
            if (!hastaptr) {
                fprintf(stderr, "Error línea %d: PARA requiere HASTA.\n", linea_actual);
                int fp = encontrar_fin_para(linea_num, nivel);
                if (fp != -1) linea_num = fp + 1;
                else linea_num++;
                continue;
            }
            p = hastaptr + 6;
            while(*p == ' ') p++;
            int fin = 0;
            if (*p == '$') {
                p++;
                char var_hasta[MAX_NOMBRE];
                j = 0;
                while (es_alnum(*p) && j < MAX_NOMBRE-1)
                    var_hasta[j++] = *p++;
                var_hasta[j] = '\0';
                fin = get_var_valor_global(var_hasta);
            } else {
                fin = atoi(p);
            }
            int paso = 1;
            char *pasoptr = strstr(linea, "PASO");
            if (pasoptr) {
                p = pasoptr + 4;
                while(*p == ' ') p++;
                if (*p == '$') {
                    p++;
                    char var_paso[MAX_NOMBRE];
                    j = 0;
                    while (es_alnum(*p) && j < MAX_NOMBRE-1) var_paso[j++] = *p++;
                    var_paso[j] = '\0';
                    paso = get_var_valor_global(var_paso);
                } else {
                    paso = atoi(p);
                    if (paso == 0) paso = 1;
                }
            }
            int valor = get_var_valor_global(var_nombre);
            int direction_ok = 1;
            if (paso > 0 && valor > fin) direction_ok = 0;
            if (paso < 0 && valor < fin) direction_ok = 0;
            if (direction_ok && para_stack_ptr < MAX_NESTING) {
                para_stack[para_stack_ptr].nivel = nivel;
                strncpy(para_stack[para_stack_ptr].var_nombre, var_nombre, MAX_NOMBRE-1);
                para_stack[para_stack_ptr].var_nombre[MAX_NOMBRE-1] = '\0';
                para_stack[para_stack_ptr].fin = fin;
                para_stack[para_stack_ptr].paso = paso;
                para_stack[para_stack_ptr].linea_inicio = linea_num;
                para_stack[para_stack_ptr].linea_fin = encontrar_fin_para(linea_num, nivel);
                para_stack_ptr++;
                linea_num++;
            } else {
                int fp = encontrar_fin_para(linea_num, nivel);
                if (fp != -1)
                    linea_num = fp + 1;
                else
                    linea_num++;
            }
            continue;
        }
        
        if (strncmp(linea, "REALIZAR", 8) == 0) {
            int nivel = 0;
            char *p = linea + 8;
            while(*p == ' ') p++;
            if (*p == '{') {
                p++;
                nivel = atoi(p);
                while(*p && *p != '}') p++;
                if (*p == '}') p++;
            }
            int ya_en_stack = 0;
            for (int i = 0; i < proceder_stack_ptr; i++) {
                if (proceder_stack[i].nivel == nivel && proceder_stack[i].linea_inicio == linea_num) {
                    ya_en_stack = 1;
                    break;
                }
            }
            int fin_proceder = encontrar_fin_proceder(linea_num, nivel);
            if (fin_proceder == -1) {
                fprintf(stderr, "Error línea %d: REALIZAR sin MIENTRAS.\n", linea_actual);
                linea_num++;
                continue;
            }
            if (!ya_en_stack && proceder_stack_ptr < MAX_NESTING) {
                proceder_stack[proceder_stack_ptr].linea_inicio = linea_num;
                proceder_stack[proceder_stack_ptr].linea_fin = fin_proceder;
                proceder_stack[proceder_stack_ptr].nivel = nivel;
                proceder_stack_ptr++;
            }
            linea_num++;
            continue;
        }
        
        if (strncmp(linea, "MIENTRAS", 8) == 0 && strchr(linea, '(') && proceder_stack_ptr > 0) {
            int nivel = 0;
            char *p = linea + 8;
            while(*p == ' ') p++;
            if (*p == '{') {
                p++;
                nivel = atoi(p);
            }

            int exito, resultado;
            char condicion[MAX_LINEA] = "";
            
            const char *p_abre = strchr(linea, '(');
            const char *p_cierra = p_abre ? strchr(p_abre + 1, ')') : NULL;
            
            if (p_abre && p_cierra && p_cierra > p_abre) {
                int len = (int)(p_cierra - p_abre - 1);
                if (len > 0 && len < MAX_LINEA - 1) {
                    memcpy(condicion, p_abre + 1, len);
                    condicion[len] = '\0';
                }
            }
            
            char *inicio = condicion;
            while (*inicio == ' ' || *inicio == '\t') inicio++;
            char *fin = inicio + strlen(inicio) - 1;
            while (fin > inicio && (*fin == ' ' || *fin == '\t' || *fin == '\r')) {
                *fin = '\0';
                fin--;
            }
            if (inicio != condicion) {
                memmove(condicion, inicio, strlen(inicio) + 1);
            }
            
            char cond_eval[MAX_LINEA + 2];
            cond_eval[0] = ' ';
            strcpy(cond_eval + 1, condicion);
            
            resultado = evaluar_condicion(cond_eval, &exito);
            if (!exito) {
                fprintf(stderr, "Error línea %d.\n", linea_actual);
                for (int i = proceder_stack_ptr - 1; i >= 0; i--) {
                    if (proceder_stack[i].nivel == nivel) {
                        for (int j = i; j < proceder_stack_ptr - 1; j++)
                            proceder_stack[j] = proceder_stack[j + 1];
                        proceder_stack_ptr--;
                        break;
                    }
                }
                linea_num++;
                continue;
            }
            if (resultado) {
                for (int i = proceder_stack_ptr - 1; i >= 0; i--) {
                    if (proceder_stack[i].nivel == nivel) {
                        linea_num = proceder_stack[i].linea_inicio + 1;
                        break;
                    }
                }
            } else {
                for (int i = proceder_stack_ptr - 1; i >= 0; i--) {
                    if (proceder_stack[i].nivel == nivel) {
                        for (int j = i; j < proceder_stack_ptr - 1; j++)
                            proceder_stack[j] = proceder_stack[j + 1];
                        proceder_stack_ptr--;
                        break;
                    }
                }
                linea_num++;
            }
            continue;
        }
        
        if (comienza_con(linea, "MIENTRAS") == 0 && comienza_con(linea, "HACER")) {
            if (mientras_stack_ptr < MAX_NESTING) {
                mientras_stack[mientras_stack_ptr].linea_inicio = linea_num;
                mientras_stack[mientras_stack_ptr].linea_fin = encontrar_fin_mientras(linea_num, 0);
                mientras_stack_ptr++;
            }

            int exito = 0;
            int resultado = 0;
            char condicion[MAX_LINEA] = "";    
            condicion[0] = '\0';
            
            const char *p_abre = strchr(linea, '(');
            const char *p_cierra = p_abre ? strchr(p_abre + 1, ')') : NULL;
            
            if (p_abre && p_cierra && p_cierra > p_abre) {
                int len = (int)(p_cierra - p_abre - 1);
                if (len > 0 && len < MAX_LINEA - 1) {
                    memcpy(condicion, p_abre + 1, len);
                    condicion[len] = '\0'; 
                }
            }

            limpiar_string(condicion);
            char cond_eval[MAX_LINEA + 2];
            cond_eval[0] = ' ';
            strcpy(cond_eval + 1, condicion);
            resultado = evaluar_condicion(cond_eval, &exito);
            
            if (!exito) {
                fprintf(stderr, "Error línea %d: Condición inválida.\n", linea_actual);
                if (mientras_stack_ptr > 0) mientras_stack_ptr--;
                int fin = encontrar_fin_mientras(linea_num, 0);
                if (fin != -1)
                    linea_num = fin + 1;
            	else
                    linea_num++;
                continue;
            }
    
            if (resultado) {
                linea_num++;
            } else {
                int fin = encontrar_fin_mientras(linea_num, 0);
                if (fin != -1)
                    linea_num = fin + 1;
                else
                    linea_num++;
        
                if (mientras_stack_ptr > 0) mientras_stack_ptr--;
            }
            continue;
        }
        
        if (comienza_con(linea, "SI") && !comienza_con(linea, "SINO")) {
            if (!strstr(linea, "ENTONCES")) {
                fprintf(stderr, "Error de sintaxis en línea %d: Falta 'ENTONCES' en declaración SI.\n", linea_actual);
                fprintf(stderr, "Formato correcto: SI (condición) ENTONCES.\n");
                if (modo_estricto) exit(1);
                linea_num++;
                continue;
            }

            int exito = 0;
            int resultado = 0;
            char condicion[MAX_LINEA] = "";

            condicion[0] = '\0';
            const char *p_abre = strchr(linea, '(');
            const char *p_cierra = p_abre ? strchr(p_abre + 1, ')') : NULL;
            
            if (p_abre && p_cierra && p_cierra > p_abre) {
                int len = (int)(p_cierra - p_abre - 1);
                if (len > 0 && len < MAX_LINEA - 1) {
                    memcpy(condicion, p_abre + 1, len);
                    condicion[len] = '\0';
                }
            }
            
            char *inicio = condicion;
            while (*inicio == ' ' || *inicio == '\t') inicio++;
            char *fin = inicio + strlen(inicio) - 1;
            while (fin > inicio && (*fin == ' ' || *fin == '\t' || *fin == '\r')) {
                *fin = '\0';
                fin--;
            }
            if (inicio != condicion) {
                memmove(condicion, inicio, strlen(inicio) + 1);
            }
            
            char cond_eval[MAX_LINEA + 2];
            cond_eval[0] = ' ';
            strcpy(cond_eval + 1, condicion);
            resultado = evaluar_condicion(cond_eval, &exito);
            
            int fin_si = encontrar_fin_si(linea_num);
            if (!exito) {
                fprintf(stderr, "Error línea %d: Condición inválida.\n", linea_actual);
                if (fin_si != -1)
                    linea_num = fin_si + 1;
                else
                    linea_num++;
                continue;
            }
            
            if (resultado) {
                if (fin_si == -1) {
                    fprintf(stderr, "Error en línea %d: Falta 'FIN SI' para cerrar el bloque SI.\n", linea_actual);
                    if (modo_estricto) exit(1);
                    for (int b = linea_num + 1; b < num_lineas_programa; b++) {
                        char lb[MAX_LINEA]; strncpy(lb, lineas_programa[b], MAX_LINEA-1); lb[MAX_LINEA-1]='\0';
                        limpiar_string(lb); remover_comentario(lb);
                        if (strncmp(lb, "FIN SI", 6) == 0) { linea_num = b + 1; break; }
                    }
                    continue;
                }

                if (si_stack_ptr < MAX_NESTING) {
                    si_stack[si_stack_ptr].linea_inicio = linea_num;
                    si_stack[si_stack_ptr].linea_fin = fin_si;
                    si_stack_ptr++;
                }
                linea_num++;
            } else {
                int sino_linea = encontrar_sino(linea_num);
                if (sino_linea != -1)
                    linea_num = sino_linea + 1;
                else if (fin_si != -1)
                    linea_num = fin_si + 1;
                else
                    linea_num++;
            }
            continue;
        }
        
        if (comienza_con(linea, "SINO SI")) {
            if (!strstr(linea, "ENTONCES")) {
                fprintf(stderr, "Error de sintaxis en línea %d: Falta 'ENTONCES' en declaración SINO SI.\n", linea_actual);
                fprintf(stderr, "Formato correcto: SINO SI (condición) ENTONCES.\n");
                if (modo_estricto) exit(1);
                linea_num++;
                continue;
            }

            if (si_stack_ptr > 0) {
                int fin_si = si_stack[si_stack_ptr - 1].linea_fin;
                si_stack_ptr--;
                if (fin_si != -1)
                    linea_num = fin_si + 1;
                else
                    linea_num++;
            } else
                linea_num++;
            continue;
        }

        if (comienza_con(linea, "SINO") && !comienza_con(linea, "SINO SI")) {
            if (si_stack_ptr > 0) {
                int fin_si = si_stack[si_stack_ptr - 1].linea_fin;
                si_stack_ptr--;
                if (fin_si != -1)
                    linea_num = fin_si + 1;
                else
                    linea_num++;
            } else
                linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "SEGUN CASO")) {
            int exito;
            double valor = 0;
            char *ptr = linea + 10;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            if (*ptr != '(') {
                fprintf(stderr, "Error línea %d: SEGUN CASO requiere paréntesis: SEGUN CASO ($variable).\n", linea_actual);
                int fin = encontrar_fin_segun(linea_num);
                if (fin != -1) linea_num = fin + 1;
                else linea_num++;
                continue;
            }
            char *inicio = strchr(linea, '(');
            char *fin = strchr(linea, ')');
            if (inicio && fin) {
                char expresion[MAX_LINEA];
                int len = fin - inicio - 1;
                if (len > 0 && len < MAX_LINEA) {
                    strncpy(expresion, inicio + 1, len);
                    expresion[len] = '\0';
                    valor = evaluar_expresion_completa(expresion, &exito);
                }
            }
            if (segun_stack_ptr < MAX_NESTING) {
                segun_stack[segun_stack_ptr].linea_inicio = linea_num;
                segun_stack[segun_stack_ptr].linea_fin = encontrar_fin_segun(linea_num);
                segun_stack[segun_stack_ptr].valor_variable = (int)valor;
                segun_stack[segun_stack_ptr].caso_encontrado = 0;
                segun_stack_ptr++;
            }
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "CASO")) {
            if (segun_stack_ptr > 0) {
                SegunBloque *bloque = &segun_stack[segun_stack_ptr - 1];
                char *ptr = linea + 4;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                int valor_caso = atoi(ptr);
                if (bloque->caso_encontrado) {
                    int i = linea_num + 1;
                    while (i < num_lineas_programa) {
                        char l[MAX_LINEA];
                        strncpy(l, lineas_programa[i], MAX_LINEA - 1);
                        l[MAX_LINEA - 1] = '\0';
                        limpiar_string(l);
                        remover_comentario(l);
                        if (comienza_con(l, "CASO") || comienza_con(l, "POR DEFECTO") ||
                            comienza_con(l, "FIN SEGUN")) {
                            linea_num = i;
                            break;
                        }
                        i++;
                    }
                    if (i >= num_lineas_programa) linea_num = bloque->linea_fin;
                    continue;
                }
                if (bloque->valor_variable == valor_caso) {
                    bloque->caso_encontrado = 1;
                    linea_num++;
                    continue;
                }
                int i = linea_num + 1;
                while (i < num_lineas_programa) {
                    char l[MAX_LINEA];
                    strncpy(l, lineas_programa[i], MAX_LINEA - 1);
                    l[MAX_LINEA - 1] = '\0';
                    limpiar_string(l);
                    remover_comentario(l);
                    if (comienza_con(l, "CASO") || comienza_con(l, "POR DEFECTO") ||
                        comienza_con(l, "FIN SEGUN")) {
                        linea_num = i;
                        break;
                    }
                    i++;
                }
                if (i >= num_lineas_programa) linea_num = bloque->linea_fin;
                continue;
            }
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "POR DEFECTO")) {
            if (segun_stack_ptr > 0) {
                SegunBloque *bloque = &segun_stack[segun_stack_ptr - 1];
                if (bloque->caso_encontrado) {
                    int i = linea_num + 1;
                    while (i < num_lineas_programa) {
                        char l[MAX_LINEA];
                        strncpy(l, lineas_programa[i], MAX_LINEA - 1);
                        l[MAX_LINEA - 1] = '\0';
                        limpiar_string(l);
                        remover_comentario(l);
                        if (comienza_con(l, "FIN SEGUN")) {
                            linea_num = i;
                            break;
                        }
                        i++;
                    }
                    if (i >= num_lineas_programa) linea_num = bloque->linea_fin;
                    continue;
                }
                bloque->caso_encontrado = 1;
                linea_num++;
                continue;
            }
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "CORTE")) {
            int realizado = 0;
    
            if (segun_stack_ptr > 0) {
                int fin_segun = segun_stack[segun_stack_ptr - 1].linea_fin;
                segun_stack_ptr--;
                if (fin_segun != -1) {
                    linea_num = fin_segun + 1;
                    realizado = 1;
                }
            }
    
            if (!realizado && mientras_stack_ptr > 0) {
                int fin_mientras = mientras_stack[mientras_stack_ptr - 1].linea_fin;
                if (fin_mientras != -1) {
                    linea_num = fin_mientras + 1;
                    mientras_stack_ptr--;
                    realizado = 1;
                }
            }
 
            if (!realizado && para_stack_ptr > 0) {
                para_stack_ptr--;
                int fin_para = para_stack[para_stack_ptr].linea_fin;
                if (fin_para != -1) {
                    linea_num = fin_para + 1;
                    realizado = 1;
                }
            }
    
            if (!realizado && proceder_stack_ptr > 0) {
                int fin_proceder = proceder_stack[proceder_stack_ptr - 1].linea_fin;
                proceder_stack_ptr--;
                if (fin_proceder != -1) {
                    linea_num = fin_proceder + 1;
                    realizado = 1;
                }
            }
    
            if (!realizado && en_subprograma && sub_stack_ptr > 0) {
                int nivel = 0;
                for (int b = linea_num + 1; b < num_lineas_programa; b++) {
                    char lb[MAX_LINEA];
                    strncpy(lb, lineas_programa[b], MAX_LINEA - 1);
                    lb[MAX_LINEA - 1] = '\0';
                    limpiar_string(lb);
                    remover_comentario(lb);
            
                    if (comienza_con(lb, "FIN SUBPROGRAMA")) {
                        if (nivel == 0) {
                            linea_num = b;
                            realizado = 1;
                            break;
                        }
                        nivel--;
                    }
                    else if (comienza_con(lb, "SUBPROGRAMA")) {
                        nivel++;
                }
            }
        }
    
        if (!realizado) {
            fprintf(stderr, "Error línea %d: CORTE fuera de un bucle, SEGUN o SUBPROGRAMA.\n", linea_actual);
            fprintf(stderr, "CORTE válido en: MIENTRAS, PARA, REALIZAR, SEGUN CASO, SUBPROGRAMA.\n");
            linea_num++;
        }   
        continue;
        }
        
        if (comienza_con(linea, "CONFIGURARPIN")) {
            procesar_gpio_configurar(linea + 13);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "ESTADOPIN")) {
            procesar_gpio_estado_pin(linea + 9);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEERPIN")) {
            procesar_gpio_leer(linea + 7);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "ESCRIBIR") || comienza_con(linea, "MOSTRAR")) {
            char *texto;
            if (comienza_con(linea, "ESCRIBIR")) {
                texto = linea + 8;
            } else {
                texto = linea + 7;
            }
            while (*texto == ' ' || *texto == '\t') texto++;
            procesar_escribir(texto);
            fflush(stdout);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "CURSOR") || comienza_con(linea, "POSICIONAR")) {
            const char *ptr = strchr(linea, '(');
            if (ptr) {
                ptr++;
                char buffer[MAX_LINEA];
                strncpy(buffer, ptr, MAX_LINEA - 1);
                buffer[MAX_LINEA - 1] = '\0';
        
                char *fin = strchr(buffer, ')');
                if (fin) *fin = '\0';
        
                char *coma = strchr(buffer, ',');
                if (coma) {
                    *coma = '\0';
                    int fila = atoi(buffer);
                    int columna = atoi(coma + 1);
            
                    if (fila > 0 && columna > 0) {
                        nico_posicionar_cursor(fila, columna);
                    }
                }
            }
            linea_num++;
            continue;
        }       
 
        if (comienza_con(linea, "ESCRIBIRARCHIVO")) {
            procesar_escribirarchivo(linea + 15);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEERARCHIVO")) {
            procesar_leerarchivo(linea + 11);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEERLINEA")) {
            procesar_leerlinea(linea + 9);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "ABRIRARCHIVO")) {
            procesar_abrirarchivo(linea + 13);
            linea_num++;
            continue;
        }        

        if (comienza_con(linea, "CERRARARCHIVO")) {
            procesar_cerrararchivo(linea + 13);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEER")) {
            char *argumento = linea + 4;
            while (*argumento == ' ' || *argumento == '\t') argumento++;
            procesar_leer(argumento);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEERHASTA")) {
            const char *apertura = strchr(linea, '(');
            if (apertura) {
                procesar_leerhasta(apertura);
            }
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEERCARACTER")) {
            procesar_leercaracter(linea + 14);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "COLORTEXTO")) {
            const char *apertura = strchr(linea, '(');
            if (apertura) {
                procesar_colortexto(apertura);
            }
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "COLORFONDO")) {
            const char *apertura = strchr(linea, '(');
            if (apertura) {
                procesar_colorfondo(apertura);
            }
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "TEXTONEGRITA")) {
            const char *ptr = linea + 12;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            if (*ptr == '(' || *ptr == ')') {
                fprintf(stderr, "Error línea %d: TEXTONEGRITA no lleva paréntesis. Uso: TEXTONEGRITA\n", linea_num + 1);
                exit(1);
            }
            procesar_textonegrita(NULL);
            linea_num++; continue;
        }

        if (comienza_con(linea, "TEXTOCURSIVA")) {
            const char *ptr = linea + 12;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            if (*ptr == '(' || *ptr == ')') {
                fprintf(stderr, "Error línea %d: TEXTOCURSIVA no lleva paréntesis. Uso: TEXTOCURSIVA\n", linea_num + 1);
                exit(1);
            }
            procesar_textocursiva(NULL);
            linea_num++; continue;
        }

        if (comienza_con(linea, "TEXTOSUBRAYADO")) {
            const char *ptr = linea + 14;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            if (*ptr == '(' || *ptr == ')') {
                fprintf(stderr, "Error línea %d: TEXTOSUBRAYADO no lleva paréntesis. Uso: TEXTOSUBRAYADO\n", linea_num + 1);
                exit(1);
            }
            procesar_textosubrayado(NULL);
            linea_num++; continue;
        }

        if (comienza_con(linea, "RESETTEXTO")) {
            const char *ptr = linea + 10;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            if (*ptr == '(' || *ptr == ')') {
                fprintf(stderr, "Error línea %d: RESETTEXTO no lleva paréntesis. Uso: RESETTEXTO\n", linea_num + 1);
                exit(1);
            }
            procesar_textoreset(NULL);
            linea_num++; continue;
        }

        if (comienza_con(linea, "RESETCOLOR")) {
            const char *ptr = linea + 10;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            if (*ptr == '(' || *ptr == ')') {
                fprintf(stderr, "Error línea %d: RESETCOLOR no lleva paréntesis. Uso: RESETCOLOR\n", linea_num + 1);
                exit(1);
            }
            procesar_resetcolor(NULL);
            linea_num++; continue;
        }
        
        if (strncmp(linea, "COPIARTEXTO(", 12) == 0 || strncmp(linea, "CONCATENARTEXTO(", 16) == 0 ||
            strncmp(linea, "MAYUSCULAS(", 11) == 0 || strncmp(linea, "MINUSCULAS(", 11) == 0 ||
            strncmp(linea, "RECORTARTEXTO(", 14) == 0 || strncmp(linea, "REEMPLAZARTEXTO(", 16) == 0 ||
            strncmp(linea, "ENTEROATEXTO(", 13) == 0 || strncmp(linea, "DECIMALATEXTO(", 14) == 0 ||
            strncmp(linea, "CARACTERATEXTO(", 15) == 0 || strncmp(linea, "REPETIRTEXTO(", 13) == 0 ||
            strncmp(linea, "EXTRAERTEXTO(", 13) == 0 || strncmp(linea, "DIVIDIRTEXTO(", 13) == 0) {
            procesar_funcion_texto(linea);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LIMPIARPANTALLA")) {
            procesar_limpiarpantalla();
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "ESPERAR")) {
            const char *ptr = linea + 7;
            while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
            
            if (!strchr(ptr, ',')) {
                fprintf(stderr, "Error línea %d: Formato inválido. Uso correcto: ESPERAR(valor, UNIDAD)\n", linea_num + 1);
                exit(1);
            }
            
            char *argumento = linea + 7;
            while (*argumento == ' ' || *argumento == '\t') argumento++;
            procesar_esperar(argumento);
            linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "SISTEMA")) {
            char *argumento = linea + 7;
            while (*argumento == ' ' || *argumento == '\t') argumento++;
            procesar_sistema(argumento);
            linea_num++;
            continue;
        }
        
        linea_num++;
    }
    
    nico_gpio_cleanup();
    scope_actual = -1;
    en_funcion = 0;
    cerrar_todos_los_archivos();
    fprintf(stderr, "> Programa '%s' finalizado.\n", nombre_programa);
    limpiar_memoria_completa();

    
    return 0;
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
    #include <windows.h>
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    if (argc == 2) {
        if (cargar_archivo_en_memoria(argv[1]) != 0) {
            fprintf(stderr, "Error: No se pudo abrir '%s'.\n", argv[1]);
            return 1;
        }

        scope_actual = -1;
        for (int i = 0; i < MAX_SCOPES; i++) {
            scopes_locales[i].num_variables = 0;
        }
        funcion_stack_ptr = 0;
       
        ejecutar_programa_cargado();
        limpiar_memoria_completa();
        return 0;
    }
    
    fprintf(stderr, "\n> Intérprete del lenguaje Nico v1.0\n");
    fprintf(stderr, "> Modo interactivo. Escribí SALIR para terminar.\n");
    fprintf(stderr, "> Escribí ? para ver ayuda.\n");
    
    char comando[MAX_LINEA];
    int archivo_cargado = 0;
    
    while (1) {
        mostrar_prompt();
        if (!fgets(comando, MAX_LINEA, stdin)) break;
        limpiar_string(comando);
        if (!strlen(comando)) continue;
        
        if (strcmp(comando, "?") == 0) {
            comando_ayuda();
            continue;
        }
        
        if (comienza_con(comando, "USAR")) {
            if (archivo_cargado && num_lineas_programa > 0) {
                fprintf(stderr, "Error: Ya hay un programa cargado.\n");
                continue;
            }
            char *archivo = comando + 4;
            while (*archivo == ' ' || *archivo == '\t') archivo++;
            char *ext = strrchr(archivo, '.');
            if (!ext || strcmp(ext, ".nico") != 0) {
                fprintf(stderr, "Error: El archivo debe tener extensión .nico\n");
                continue;
            }
            limpiar_memoria_completa();
            if (cargar_archivo_en_memoria(archivo) != 0) {
                fprintf(stderr, "Error: No se pudo abrir '%s'.\n", archivo);
                continue;
            }
            fprintf(stderr, "> Archivo '%s' cargado con éxito.\n", archivo);
            archivo_cargado = 1;
            continue;
        }
        
        if (comienza_con(comando, "CORRER")) {
            if (!archivo_cargado || num_lineas_programa == 0) {
                fprintf(stderr, "Error: No hay programa cargado.\n");
                continue;
            }
            ejecutar_programa_cargado();
            continue;
        }
        
        if (comienza_con(comando, "CERRAR")) {
            if (!archivo_cargado || num_lineas_programa == 0) {
                fprintf(stderr, "Error: No hay programa cargado\n");
                continue;
            }
            limpiar_memoria_completa();
            archivo_cargado = 0;
            fprintf(stderr, "> Programa cerrado.\n");
            continue;
        }
        
        if (comienza_con(comando, "RANGOS")) {
            comando_rangos();
            continue;
        }
        
        if (comienza_con(comando, "SALIR")) {
            limpiar_memoria_completa();
            fprintf(stderr, "> Intérprete terminado.\n");
            break;
        }
        
        fprintf(stderr, "Comando desconocido. Escribí ? para ayuda.\n");
    }
    
    return 0;
}
