#include "nico.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>

const CmdEntry dispatch_table[] = {
    { "RESETTEXTO",          cmd_resettexto          },
    { "RESETCOLOR",          cmd_resetcolor          },
    { "LIMPIARPANTALLA",     cmd_limpiarpantalla     },
    { "ESCRIBIR",            cmd_escribir            },
    { "MOSTRAR",             cmd_escribir            }, 
    { "COLORTEXTO",          cmd_colortexto          },
    { "COLORFONDO",          cmd_colorfondo          },
    { "TEXTONEGRITA",        cmd_textonegrita        },
    { "TEXTOCURSIVA",        cmd_textocursiva        },
    { "TEXTOSUBRAYADO",      cmd_textosubrayado      },
    { "CALCULAR EN",         cmd_calcular            },
    { "RESULTADO EN",        cmd_resultado           },
    { "ASIGNAR EN",          cmd_asignar             },
    { "VARIABLE ENTERA SIN SIGNO",  cmd_var_entera_sin   },
    { "VARIABLE DECIMAL SIN SIGNO", cmd_var_decimal_sin  },
    { "VARIABLE CARACTER SIN SIGNO",cmd_var_caracter_sin },
    { "VARIABLE ENTERA",         cmd_var_entera        },
    { "VARIABLE DECIMAL",        cmd_var_decimal       },
    { "VARIABLE TEXTO",          cmd_var_texto         },
    { "VARIABLE CARACTER",       cmd_var_caracter      },
    { "DECLARAR VARIABLE ENTERA",            cmd_var_entera        },
    { "DECLARAR VARIABLE DECIMAL",           cmd_var_decimal       },
    { "DECLARAR VARIABLE TEXTO",             cmd_var_texto         },
    { "DECLARAR VARIABLE CARACTER",          cmd_var_caracter      },
    { "DECLARAR VARIABLE ENTERA SIN SIGNO",  cmd_var_entera_sin    },
    { "DECLARAR VARIABLE DECIMAL SIN SIGNO", cmd_var_decimal_sin   },
    { "DECLARAR VARIABLE CARACTER SIN SIGNO",cmd_var_caracter_sin  },
    { "CONSTANTE ENTERA SIN SIGNO",   cmd_constante_entera_sin          },
    { "DECLARAR CONSTANTE ENTERA SIN SIGNO", cmd_constante_entera_sin   },
    { "CONSTANTE DECIMAL SIN SIGNO",  cmd_constante_decimal_sin },
    { "DECLARAR CONSTANTE DECIMAL SIN SIGNO", cmd_constante_decimal_sin },
    { "CONSTANTE ENTERA",             cmd_constante_entera              },
    { "DECLARAR CONSTANTE ENTERA",    cmd_constante_entera              },
    { "CONSTANTE DECIMAL",            cmd_constante_decimal             },
    { "DECLARAR CONSTANTE DECIMAL",   cmd_constante_decimal             },
    { "CONSTANTE TEXTO",              cmd_constante_texto               },
    { "DECLARAR CONSTANTE TEXTO",     cmd_constante_texto               },
    { "LLAMAR A",                     cmd_llamar_a                      },
    { "SUBPROGRAMA",       cmd_subprograma       },
    { "FIN SUBPROGRAMA",   cmd_fin_subprograma   },
    { "SINO SI",           cmd_sino_si         },
    { "SINO",              cmd_sino            },
    { "SI",                cmd_si              },
    { "FIN SI",            cmd_fin_si          },
    { "SEGUN CASO",      cmd_segun_caso      },
    { "POR DEFECTO",     cmd_por_defecto     },
    { "FIN SEGUN",       cmd_fin_segun       },
    { "CASO",            cmd_caso            },
    { "FIN FUNCION",       cmd_fin_funcion       },
    { "FUNCION",           cmd_funcion           },
    { "RETORNAR",          cmd_retornar          },
    { "LISTA ENTERA SIN SIGNO",      cmd_lista_entera_sin     },
    { "DECLARAR LISTA ENTERA SIN SIGNO", cmd_lista_entera_sin },
    { "LISTA DECIMAL SIN SIGNO",     cmd_lista_decimal_sin    },
    { "DECLARAR LISTA DECIMAL SIN SIGNO", cmd_lista_decimal_sin },
    { "LISTA CARACTER SIN SIGNO",    cmd_lista_caracter_sin   },
    { "DECLARAR LISTA CARACTER SIN SIGNO", cmd_lista_caracter_sin },
    { "LISTA ENTERA",                cmd_lista_entera         },
    { "DECLARAR LISTA ENTERA",       cmd_lista_entera         },
    { "LISTA DECIMAL",               cmd_lista_decimal        },
    { "DECLARAR LISTA DECIMAL",      cmd_lista_decimal        },
    { "LISTA CARACTER",              cmd_lista_caracter       },
    { "DECLARAR LISTA CARACTER",     cmd_lista_caracter       },
    { "MATRIZ ENTERA SIN SIGNO",     cmd_matriz_entera_sin    },
    { "DECLARAR MATRIZ ENTERA SIN SIGNO", cmd_matriz_entera_sin },
    { "MATRIZ DECIMAL SIN SIGNO",    cmd_matriz_decimal_sin   },
    { "DECLARAR MATRIZ DECIMAL SIN SIGNO", cmd_matriz_decimal_sin },
    { "MATRIZ CARACTER SIN SIGNO",   cmd_matriz_caracter_sin  },
    { "DECLARAR MATRIZ CARACTER SIN SIGNO", cmd_matriz_caracter_sin },
    { "MATRIZ ENTERA",               cmd_matriz_entera        },
    { "DECLARAR MATRIZ ENTERA",      cmd_matriz_entera        },
    { "MATRIZ DECIMAL",              cmd_matriz_decimal       },
    { "DECLARAR MATRIZ DECIMAL",     cmd_matriz_decimal       },
    { "MATRIZ CARACTER",             cmd_matriz_caracter      },
    { "DECLARAR MATRIZ CARACTER",    cmd_matriz_caracter      },
    { "VARIABLE ARCHIVO",            cmd_var_archivo          },
    { "DECLARAR VARIABLE ARCHIVO",   cmd_var_archivo          },
    { "FIN MIENTRAS",      cmd_fin_mientras      },
    { "FIN PARA",          cmd_fin_para          },
    { "MIENTRAS",          cmd_mientras          },
    { "REALIZAR",          cmd_realizar          },
    { "PARA",              cmd_para              },
    { "SALTAR A",          cmd_saltar_a          },
    { "CORTE",             cmd_corte             },
    { "CONFIGURARPIN",     cmd_configurar_pin      },
    { "ESTADOPIN",         cmd_estado_pin          },
    { "LEERPIN",           cmd_leer_pin            },
    { NULL, NULL }
};

int dispatch_command(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *ptr = linea;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    for (int i = 0; dispatch_table[i].keyword != NULL; i++) {
        if (comienza_con(ptr, dispatch_table[i].keyword)) {
            return dispatch_table[i].handler(ptr, ctx, linea_actual);
        }
    }
    return -2;
}

int ejecutar_bloque(CtxBloque *ctx) {
    while (ctx->linea_num < num_lineas_programa && 
      (ctx->linea_limite < 0 || ctx->linea_num <= ctx->linea_limite)) {   
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[ctx->linea_num], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        saltar_espacios_inplace(linea);
        remover_comentario(linea);

        int linea_actual = ctx->linea_num + 1;
        
        if (!strlen(linea)) {
            ctx->linea_num++;
            continue;
        }
        
        if (!ctx->inicio_encontrado) {
            if (comienza_con(linea, "PROGRAMA")) {
                ctx->inicio_encontrado = 1;
            } else {
                fprintf(stderr, "Error línea %d.\n", linea_actual);
                return -1;
            }
            ctx->linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "FINAL")) {
            if (!fin_principal_encontrado) {
                fprintf(stderr, "Error: El programa no tiene FIN PRINCIPAL antes de FINAL.\n");
                return -1;
            }
            nico_gpio_cleanup();
            cerrar_todos_los_archivos();
            return 0;
        }
        
        if (comienza_con(linea, "BLOQUE PRINCIPAL")) {
            if (ctx->en_bloque_principal) {
                fprintf(stderr, "Error línea %d: Ya hay un BLOQUE PRINCIPAL abierto.\n", linea_actual);
                return -1;
            }
            ctx->en_bloque_principal = 1;
            ctx->fase_declaraciones = 0;
        
            {
                int inicio_main = ctx->linea_num + 1;
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

            ctx->linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "FIN PRINCIPAL")) {
            if (!ctx->en_bloque_principal) {
                fprintf(stderr, "Error línea %d: FIN PRINCIPAL sin BLOQUE PRINCIPAL.\n", linea_actual);
                return -1;
            }
            ctx->en_bloque_principal = 0;
            fin_principal_encontrado = 1;
            ctx->linea_num++;
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
        

        if (en_subprograma && (comienza_con(linea, "VARIABLE") || comienza_con(linea, "DECLARAR"))) {
            fprintf(stderr, "Error línea %d: Los SUBPROGRAMAS no admiten declarar variables locales.\n", linea_actual);
            fprintf(stderr, "Use variables globales declaradas antes del BLOQUE PRINCIPAL.\n");
            return -1;
        }
        
       
        if (!comienza_con(linea, "CONSTANTE") && !comienza_con(linea, "DECLARAR") &&
            !comienza_con(linea, "VARIABLE") && !comienza_con(linea, "LISTA") &&
            !comienza_con(linea, "ETIQUETA") && !comienza_con(linea, "SUBPROGRAMA") &&
            !comienza_con(linea, "FUNCION") && !comienza_con(linea, "VARIABLE ARCHIVO") &&
            !comienza_con(linea, "BLOQUE PRINCIPAL")) {
            ctx->fase_declaraciones = 0;
        }
        
        if (!ctx->en_bloque_principal && !ctx->en_subprograma && !ctx->en_funcion) {
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
            ctx->linea_num++;
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
            ctx->linea_num++;
            continue;
        }       
 
        if (comienza_con(linea, "ESCRIBIRARCHIVO")) {
            procesar_escribirarchivo(linea + 15);
            ctx->linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEERARCHIVO")) {
            procesar_leerarchivo(linea + 11);
            ctx->linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEERLINEA")) {
            procesar_leerlinea(linea + 9);
            ctx->linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "ABRIRARCHIVO")) {
            procesar_abrirarchivo(linea + 13);
            ctx->linea_num++;
            continue;
        }        

        if (comienza_con(linea, "CERRARARCHIVO")) {
            procesar_cerrararchivo(linea + 13);
            ctx->linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEER")) {
            char *argumento = linea + 4;
            while (*argumento == ' ' || *argumento == '\t') argumento++;
            procesar_leer(argumento);
            ctx->linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEERHASTA")) {
            const char *apertura = strchr(linea, '(');
            if (apertura) {
                procesar_leerhasta(apertura);
            }
            ctx->linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "LEERCARACTER")) {
            procesar_leercaracter(linea + 14);
            ctx->linea_num++;
            continue;
        }

        int res = dispatch_command(linea, ctx, linea_actual);
        if (res >= 0) continue;      
        if (res == -1) return -1;    
        
        if (strncmp(linea, "COPIARTEXTO(", 12) == 0 || strncmp(linea, "CONCATENARTEXTO(", 16) == 0 ||
            strncmp(linea, "MAYUSCULAS(", 11) == 0 || strncmp(linea, "MINUSCULAS(", 11) == 0 ||
            strncmp(linea, "RECORTARTEXTO(", 14) == 0 || strncmp(linea, "REEMPLAZARTEXTO(", 16) == 0 ||
            strncmp(linea, "ENTEROATEXTO(", 13) == 0 || strncmp(linea, "DECIMALATEXTO(", 14) == 0 ||
            strncmp(linea, "CARACTERATEXTO(", 15) == 0 || strncmp(linea, "REPETIRTEXTO(", 13) == 0 ||
            strncmp(linea, "EXTRAERTEXTO(", 13) == 0 || strncmp(linea, "DIVIDIRTEXTO(", 13) == 0) {
            procesar_funcion_texto(linea);
            ctx->linea_num++;
            continue;
        }
        
        if (comienza_con(linea, "ESPERAR")) {
            const char *ptr = linea + 7;
            while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
            
            if (!strchr(ptr, ',')) {
                fprintf(stderr, "Error línea %d: Formato inválido. Uso correcto: ESPERAR(valor, UNIDAD)\n", ctx->linea_num + 1);
                exit(1);
            }
            
            char *argumento = linea + 7;
            while (*argumento == ' ' || *argumento == '\t') argumento++;
            procesar_esperar(argumento);
            ctx->linea_num++;
            continue;
        }
     
        if (comienza_con(linea, "SISTEMA")) {
            const char *p = linea + 7; 
            while (*p == ' ' || *p == '\t' || *p == '(') p++;
    
            const char *fin = strchr(p, ')');
            if (!fin) fin = p + strlen(p);
    
            char cmd[MAX_LINEA];
            int len = fin - p;
            if (len > 0 && len < MAX_LINEA - 1) {
                strncpy(cmd, p, len);
                cmd[len] = '\0';
        
                char *start = cmd;
                char *end = start + strlen(start) - 1;
                if (strlen(start) >= 2 && 
                    ((start[0] == '"' && *end == '"') || 
                    (start[0] == '\'' && *end == '\''))) {
                        start++;
                        *end = '\0';
                }
        
                while (strlen(start) > 0 && start[strlen(start)-1] == ' ') 
                start[strlen(start)-1] = '\0';
        
                if (strlen(start) > 0) procesar_sistema(start);
            }
            ctx->linea_num++; continue;
        }
        
        ctx->linea_num++;
    }
    return error_fatal ? -1 : 0; 
}    


