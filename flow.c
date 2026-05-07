/*
 * Nico v1.0.1 - Intérprete Educativo de Scripting en Español
 * @file:         flow.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Control de flujo: búsqueda de líneas, saltos, manejo de CORTE, 
 *                SALTAR A y resolución de etiquetas.
 */
#include "nico.h"
#include <string.h>

// ENCONTRAR FIN SI
int encontrar_fin_si(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (comienza_con(linea, "SI") && strstr(linea, "ENTONCES")) {
            nivel++;
        }
        else if (comienza_con(linea, "FIN SI")) {
            nivel--;
            if (nivel == 0) return i;
        }
    }
    return -1;
}

// ENCONTRAR SINO
int encontrar_sino(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (comienza_con(linea, "SI") && strstr(linea, "ENTONCES")) {
            nivel++;
        }
        else if (comienza_con(linea, "FIN SI")) {
            nivel--;
            if (nivel == 0) return -1;
        }
        else if (nivel == 1 && comienza_con(linea, "SINO") && !comienza_con(linea, "SINO SI")) {
            return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN MIENTRAS
int encontrar_fin_mientras(int linea_inicio, int nivel) {
    int nivel_actual = nivel;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (comienza_con_keyword(linea, "MIENTRAS") && strstr(linea, "HACER")) {
            nivel_actual++;
        }

        else if (strncmp(linea, "FIN MIENTRAS", 12) == 0) {
            nivel_actual--;
            if (nivel_actual == nivel) return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN REALIZAR
int encontrar_fin_proceder(int linea_inicio, int nivel) {
    int nivel_actual = nivel;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (strncmp(linea, "REALIZAR", 8) == 0) {
            nivel_actual++;
        }
        else if (strncmp(linea, "MIENTRAS", 8) == 0) {
            nivel_actual--;
            if (nivel_actual == nivel) return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN PARA
int encontrar_fin_para(int linea_inicio, int nivel) {
    int nivel_actual = nivel;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (strncmp(linea, "PARA", 4) == 0 && strstr(linea, "HACER")) {
            nivel_actual++;
        }
        else if (strncmp(linea, "FIN PARA", 8) == 0) {
            nivel_actual--;
            if (nivel_actual == nivel) return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN SEGUN
int encontrar_fin_segun(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        
        if (comienza_con(linea, "SEGUN")) nivel++;
        if (comienza_con(linea, "FIN SEGUN")) {
            nivel--;
            if (nivel == 0) return i;
        }
    }
    return -1;
}

// ENCONTRAR SIGUIENTE CASO
int encontrar_siguiente_caso(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (comienza_con(linea, "SEGUN") && strstr(linea, "HACER")) {
            nivel++;
        }
        else if (comienza_con(linea, "FIN SEGUN")) {
            nivel--;
            if (nivel == 0) return -1;
        }
        else if (nivel >= 1 && comienza_con(linea, "CASO")) {
            return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN SUBPROGRAMA
int encontrar_fin_subprograma(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (comienza_con(linea, "SUBPROGRAMA")) {
            nivel++;
        }
        else if (comienza_con(linea, "FIN SUBPROGRAMA")) {
            nivel--;
            if (nivel == 0) return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN FUNCION
int encontrar_fin_funcion(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (comienza_con(linea, "FUNCION")) {
            nivel++;
        }
        else if (comienza_con(linea, "FIN FUNCION")) {
            nivel--;
            if (nivel == 0) return i;
        }
    }
    return -1;
}

// BUSCAR INICIO SUBPROGRAMA
int buscar_inicio_subprograma(const char *nombre) {
    for (int i = 0; i < num_subprogramas_registrados; i++) {
        if (strcmp(subprogramas_registrados[i].nombre, nombre) == 0)
            return subprogramas_registrados[i].linea_inicio;
    }
    return -1;
}
