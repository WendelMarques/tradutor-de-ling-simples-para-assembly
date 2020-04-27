/*

																	UNIVERSIDADE FEDERAL DE GOIÁS (UFG)
																		INSTITUTO DE INFORMÁTICA
																		CIÊNCIA DA COMPUTAÇÃO

															Disciplina: INF0334 – Software Básico
															Wendel Marques de Jesus Souza (201702793)*
																		[único autor]

																	RELATÓRIO DO TRABALHO FINAL

														Objetivo do programa: ler um arquivo contendo um programa na
												linguagem simples e imprimir a tradução desse programa em Assembly na tela.

*/



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int tradutor();
void remove_newline(char*);
int descobre_reg_param(int);
int if_cases(char[]);
void chamada_funcao(char[], int[], int);
int estabelece_espaco_pilha_para_params(char*);
int conta_qtd_linhas_arq();

int main ( ) {

	return tradutor();

	return 0;

}

int tradutor(){
	int i = 0, r, j;
	char line[50], palavra[50];
	int param, param1, param2, param3;
	int size;
	int indice, indice1, indice2, indice_v, constante1, constante2, nF;
	int if_test, retorno, ret_if;
	char op, c1;
	int tamanho_pilha = 0;
	int count_if = 0;
	int qtdFuncao = 0;
	int posicao_pilha;
	int inicioEndDef, fimEndDef;
	int qtd_declaracoes = 0;
	int flag_salvamento_regs;
	int numFuncao;
	int end = 1;

	int qtdLinhas = conta_qtd_linhas_arq();
	char* linhas[qtdLinhas];

	FILE *arquivo;
	arquivo = fopen("prog.slp", "r");

	if (arquivo == NULL)
		return EXIT_FAILURE;

	while(fgets(line, sizeof line, arquivo) != NULL) {
		linhas[i] = strdup(line);

		sscanf(linhas[i], "%s", palavra);
		if(strcmp(palavra, "function") == 0)
			qtdFuncao++;
		i++;
	}

	fclose(arquivo);

	i = 0;

	printf(".data\n.text\n");

	numFuncao = 0;
	while(1) {
		qtd_declaracoes = 0; //qtd de linhas em def .. enddef

		if(strcmp(palavra, "function") == 0){
			tamanho_pilha = estabelece_espaco_pilha_para_params(linhas[i]); //retorna 8 se possuir 1 param, 16 se 2 params e 24 se 3 params
			flag_salvamento_regs = tamanho_pilha;	//armazena a info anterior

			numFuncao++;
			printf("\n.globl function%d\nfunction%d:\n\tpushq %%rsp\n\tmovq %%rsp, %%rbp", numFuncao, numFuncao);
			inicioEndDef = i+1;

			//while_var
			/*Calcula o tamanho minimo necessario para armazenar as variaveis*/
			while(strcmp(palavra, "enddef") != 0){
				sscanf(linhas[i], "%s", palavra);
				i++;

				r = sscanf(linhas[i], "vet va%d size ci%d", &indice, &size);
				if (r == 2) {
					tamanho_pilha += 4 * size;
					qtd_declaracoes++;
					continue;
				}

				r = sscanf(linhas[i], "var vi%d", &indice);
				if (r == 1) {
					tamanho_pilha += 4;
					qtd_declaracoes++;
					continue;
				}
			}//while calculo pilha

			fimEndDef = i - 1;

			//caso n tenha variaveis ou n necessite salvar regs (de params)
			if(tamanho_pilha == 0)
				tamanho_pilha = 16;

			//alinha a pilha a 16
			while ((tamanho_pilha%16) != 0)
				tamanho_pilha += 4;

			printf("\n\tsubq $%d, %%rsp\n\n", tamanho_pilha);
			tamanho_pilha /= 4;

			int pilha_enderecos[tamanho_pilha];
			tamanho_pilha = tamanho_pilha*4;

			/*Para cada variavel vix, é calculado o endereco e ele é armazenado
			posição X do vetor pilha_enderecos*/
			int ini = inicioEndDef, fim = fimEndDef;

			//for_enderecos
			for( ; inicioEndDef < fimEndDef; inicioEndDef++) {
				r = sscanf(linhas[inicioEndDef], "vet va%d size ci%d", &indice, &size);
				if (r == 2) {
					pilha_enderecos[indice] = tamanho_pilha * (-1);
					tamanho_pilha = tamanho_pilha - (size * 4);
				}

				r = sscanf(linhas[inicioEndDef], "var vi%d", &indice);
				if (r == 1) {
					pilha_enderecos[indice] = tamanho_pilha * (-1);
					tamanho_pilha = tamanho_pilha - 4;
				}

				if(strcmp(palavra, "enddef") != 0)
					break;
			}

			i = fimEndDef - 1;

			end = 1; //flag q identif o termino da function

			//while_traducao
			//varre o restande do vetor(enddef .. end) e imprime em assembly o code correpondente a cada linha
			while(end) {

				if((strcmp(palavra, "endif") == 0)){
					printf("\nfim_if%d:", count_if);
					count_if++;
				}

				if((strcmp(palavra, "end") == 0)){
					end = 0;
					if(numFuncao == qtdFuncao)
						return 1;
				}

				vi_ci_mul_ci:
				r = sscanf(linhas[i], "vi%d = ci%d %c ci%d", &indice, &constante1, &op, &constante2);
				if(r == 4 && op == '*'){
					printf("\n\t#vi%d = %d * %d\n", indice, constante1, constante2);
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl $%d, %%r8d\n", constante1);
					printf("\timul $%d, %%r8d\n", constante2);
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);
					goto fim;
				}

				vi_ci_som_ci:
				r = sscanf(linhas[i], "vi%d = ci%d %c ci%d", &indice, &constante1, &op, &constante2);
				if(r == 4 && op == '+'){
					printf("\n\t#vi%d = %d + %d\n", indice, constante1, constante2);
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl $%d, %d(%%rbp)\n", constante1, posicao_pilha);
					printf("\taddl $%d, %d(%%rbp)\n", constante2, posicao_pilha);
					goto fim;
				}

				vi_vi_mul_vi:
				r = sscanf(linhas[i], "vi%d = vi%d %c vi%d", &indice, &indice1, &op, &indice2);
				if(r == 4 && op == '*'){
					printf("\n\t#vi%d = vi%d + vi%d\n", indice, indice1, indice2);
					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r8\n", posicao_pilha);
					posicao_pilha = pilha_enderecos[indice2];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovl (%%r8), %%r10d\n");
					printf("\timul (%%r9), %%r10d\n");
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r10d, %d(%%rbp)\n", posicao_pilha);
					goto fim;
				}

				vi_vi_soma_vi:
				r = sscanf(linhas[i], "vi%d = vi%d %c vi%d", &indice, &indice1, &op, &indice2);
				if(r == 4 && op == '+'){
					printf("\n\t#vi%d = vi%d * vi%d\n", indice, indice1, indice2);
					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r8\n", posicao_pilha);
					posicao_pilha = pilha_enderecos[indice2];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovl (%%r8), %%r10d\n");
					printf("\taddl (%%r9), %%r10d\n");
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r10d, %d(%%rbp)\n", posicao_pilha);
					goto fim;
				}

				vi_vi_mul_ci:
				r = sscanf(linhas[i], "vi%d = vi%d %c ci%d", &indice, &indice1, &op, &constante1);
				if(r == 4 && op == '*'){
					printf("\n\t#vi%d = vi%d * %d\n", indice, indice1, constante1);
					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovl (%%r9), %%r10d\n");
					printf("\timul $%d, %%r10d\n", constante1);
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r10d, %d(%%rbp)\n", posicao_pilha);
					goto fim;
				}

				vi_vi_soma_ci:
				r = sscanf(linhas[i], "vi%d = vi%d %c ci%d", &indice, &indice1, &op, &constante1);
				if(r == 4 && op == '+') {
					printf("\n\t#vi%d = vi%d + %d\n", indice, indice1, constante1);
					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovl (%%r9), %%r10d\n");
					printf("\taddl $%d, %%r10d\n", constante1);
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r10d, %d(%%rbp)\n", posicao_pilha);
					goto fim;
				}

				vi_ci_mul_vi:
				r = sscanf(linhas[i], "vi%d = ci%d %c vi%d", &indice, &constante1, &op, &indice1);
				if(r == 4 && op == '*'){
					printf("\n\t#vi%d = vi%d * %d\n", indice, indice1, constante1);
					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovl (%%r9), %%r10d\n");
					printf("\timul $%d, %%r10d\n", constante1);
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r10d, %d(%%rbp)\n", posicao_pilha);
					goto fim;
				}

				vi_ci_som_vi:
				r = sscanf(linhas[i], "vi%d = ci%d %c vi%d", &indice, &constante1, &op, &indice1);
				if(r == 4 && op == '+'){
					printf("\n\t#vi%d = vi%d + %d\n", indice, indice1, constante1);
					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovl (%%r9), %%r10d\n");
					printf("\taddl $%d, %%r10d\n", constante1);
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r10d, %d(%%rbp)\n", posicao_pilha);
					goto fim;
				}

				set_const:
				r = sscanf(linhas[i], "set va%d index ci%d with ci%d", &constante1, &indice_v, &constante2);
				if(r == 3) {
					printf("\n\t#vet%d[%d] = %d\n", constante1, indice_v, constante2);
					posicao_pilha = pilha_enderecos[constante1];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovabs $%d, %%r8\n", (indice_v));
					printf("\timulq $4, %%r8\n");
					printf("\taddq %%r9, %%r8\n");
					printf("\tmovl $%d, (%%r8)\n", constante2);
					goto fim;
				}

				set_vi:
				r = sscanf(linhas[i], "set va%d index ci%d with vi%d", &constante1, &indice_v, &indice);
				if(r == 3) {
					posicao_pilha = pilha_enderecos[indice];
					printf("\n\t#vet%d[%d] = vi%d\n", constante1, indice_v, indice);
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovl (%%r9), %%r10d\n");
					posicao_pilha = pilha_enderecos[constante1];
					printf("\n\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovabs $%d, %%r8\n", (indice_v));
					printf("\timulq $4, %%r8\n");
					printf("\taddq %%r9, %%r8\n");
					printf("\tmovl %%r10d, (%%r8)\n");
					goto fim;
				}

				get_vi:
				r = sscanf(linhas[i], "get va%d index ci%d to vi%d", &constante1, &indice_v, &indice);
				if(r == 3) {
					posicao_pilha = pilha_enderecos[constante1];
					printf("\n\t#vi%d = vet%d[%d]\n", indice, constante1, indice_v);
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovabs $%d, %%r8\n", (indice_v));
					printf("\timulq $4, %%r8\n");
					printf("\taddq %%r9, %%r8\n");

					printf("\n\tmovl (%%r8), %%r9d\n");
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r9d, %d(%%rbp)\n", posicao_pilha);
					goto fim;

				}

				ret_vi:
				r = sscanf(linhas[i], "return vi%d", &indice1);
				if(r == 1) {
					printf("\n\t#return vi%d\n", indice1);
					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r8", posicao_pilha);
					printf("\n\tmovl (%%r8), %%eax");
					printf("\n\tleave\n\tret\n");
					goto fim;
				}

				ret_ci:
				r = sscanf(linhas[i], "return ci%d", &constante1);
				if(r == 1) {
					printf("\n\t#return %d\n", constante1);
					printf("\tmovl $%d, %%eax\n", constante1);
					printf("\tleave\n\tret\n");
					goto fim;
				}

				ret_pi:
				r = sscanf(linhas[i], "return pi%d", &param);
				if(r == 1) {

					int reg = descobre_reg_param(param);
					printf("\n\t#return pi%d\n", param);

					switch (reg) {
						case 1:
							printf("\tmovl %%edi, %%eax\n");
							printf("\tleave\n\tret\n");
							goto fim;

						case 2:
							printf("\tmovl %%esi, %%eax\n");
							printf("\tleave\n\tret\n");
							goto fim;

						case 3:
							printf("\tmovl %%edx, %%eax\n");
							printf("\tleave\n\tret\n");
							goto fim;
					}
				}

				vi_pi_som_pi:
				r = sscanf(linhas[i], "vi%d = pi%d %c pi%d", &indice, &param1, &op, &param2);
				if(r == 4 && op == '+'){

					param1 = descobre_reg_param(param1);
					param2 = descobre_reg_param(param2);

					switch (param1) {
						case 1:

							printf("\n\tmovl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\n\tmovl %%esi, %%r8d\n");
							break;

						case 3:
							printf("\n\tmovl %%edx, %%r8d\n");
							break;
					}

					switch (param2) {
						case 1:
							printf("\taddl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\taddl  %%esi, %%r8d\n");
							break;

						case 3:
							printf("\taddl %%edx, %%r8d\n");
							break;
					}

					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);
					goto fim;
				}

				vi_pi_mul_pi:
				r = sscanf(linhas[i], "vi%d = pi%d %c pi%d", &indice, &param1, &op, &param2);
				if(r == 4 && op == '*'){
					param1 = descobre_reg_param(param1);
					param2 = descobre_reg_param(param2);

					switch (param1) {
						case 1:
							printf("\n\tmovl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\n\tmovl %%esi, %%r8d\n");
							break;

						case 3:
							printf("\n\tmovl %%edx, %%r8d\n");
							break;
					}

					switch (param2) {
						case 1:
							printf("\timul %%edi, %%r8d\n");
							break;

						case 2:
							printf("\timul  %%esi, %%r8d\n");
							break;

						case 3:
							printf("\timul %%edx, %%r8d\n");
							break;
					}

					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);


					goto fim;
				}

				vi_pi_som_vi:
				r = sscanf(linhas[i], "vi%d = pi%d %c vi%d", &indice, &param1, &op, &indice1);
				if(r == 4 && op == '+'){

					param1 = descobre_reg_param(param1);

					switch (param1) {
						case 1:
							printf("\n\tmovl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\n\tmovl %%esi, %%r8d\n");
							break;

						case 3:
							printf("\n\tmovl %%edx, %%r8d\n");
							break;
					}

					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\taddl (%%r9d), %%r8d\n");
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);

					goto fim;
				}

				vi_pi_mul_vi:
				r = sscanf(linhas[i], "vi%d = pi%d %c vi%d", &indice, &param1, &op, &indice1);
				if(r == 4 && op == '*'){

					switch (param1) {
						case 1:
							printf("\n\tmovl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\n\tmovl %%esi, %%r8d\n");
							break;

						case 3:
							printf("\n\tmovl %%edx, %%r8d\n");
							break;
					}

					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\timul (%%r9d), %%r8d\n");
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);

					goto fim;
				}

				vi_pi_som_ci:
				r = sscanf(linhas[i], "vi%d = pi%d %c ci%d", &indice, &param1, &op, &constante1);
				if(r == 4 && op == '+'){

					param1 = descobre_reg_param(param1);

					switch (param1) {
						case 1:
							printf("\n\tmovl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\n\tmovl %%esi, %%r8d\n");
							break;

						case 3:
							printf("\n\tmovl %%edx, %%r8d\n");
							break;
					}

					printf("\taddl $%d, %%r8d\n", constante1);
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);

					goto fim;
				}

				vi_pi_mul_ci:
				r = sscanf(linhas[i], "vi%d = pi%d %c ci%d", &indice, &param1, &op, &constante1);
				if(r == 4 && op == '*'){

					param1 = descobre_reg_param(param1);

					switch (param1) {
						case 1:
							printf("\n\tmovl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\n\tmovl %%esi, %%r8d\n");
							break;

						case 3:
							printf("\n\tmovl %%edx, %%r8d\n");
							break;
					}

					printf("\timul $%d, %%r8d\n", constante1);
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);

					goto fim;
				}

				vi_ci_som_pi:
				r = sscanf(linhas[i], "vi%d = ci%d %c pi%d", &indice, &constante1, &op, &param1);
				if(r == 4 && op == '+'){
					param1 = descobre_reg_param(param1);

					switch (param1) {
						case 1:
							printf("\n\tmovl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\n\tmovl %%esi, %%r8d\n");
							break;

						case 3:
							printf("\n\tmovl %%edx, %%r8d\n");
							break;
					}

					printf("\taddl $%d, %%r8d\n", constante1);
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);

					goto fim;
				}

				vi_ci_mul_pi:
				r = sscanf(linhas[i], "vi%d = ci%d %c pi%d", &indice, &constante1, &op, &param1);
				if(r == 4 && op == '*'){
					param1 = descobre_reg_param(param1);

					switch (param1) {
						case 1:
							printf("\n\tmovl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\n\tmovl %%esi, %%r8d\n");
							break;

						case 3:
							printf("\n\tmovl %%edx, %%r8d\n");
							break;
					}

					printf("\timul $%d, %%r8d\n", constante1);
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);

					goto fim;
				}

				vi_vi_som_pi:
				r = sscanf(linhas[i], "vi%d = vi%d %c pi%d", &indice, &indice1, &op, &param1);
				if(r == 4 && op == '+'){

					param1 = descobre_reg_param(param1);

					switch (param1) {
						case 1:
							printf("\n\tmovl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\n\tmovl %%esi, %%r8d\n");
							break;

						case 3:
							printf("\n\tmovl %%edx, %%r8d\n");
							break;
					}

					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\taddl (%%r9d), %%r8d\n");
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);
					goto fim;
				}

				vi_vi_mul_pi:
				r = sscanf(linhas[i], "vi%d = vi%d %c pi%d", &indice, &indice1, &op, &param1);
				if(r == 4 && op == '*'){

					param1 = descobre_reg_param(param1);

					switch (param1) {
						case 1:
							printf("\n\tmovl %%edi, %%r8d\n");
							break;

						case 2:
							printf("\n\tmovl %%esi, %%r8d\n");
							break;

						case 3:
							printf("\n\tmovl %%edx, %%r8d\n");
							break;
					}

					posicao_pilha = pilha_enderecos[indice1];
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\timul (%%r9d), %%r8d\n");
					posicao_pilha = pilha_enderecos[indice];
					printf("\tmovl %%r8d, %d(%%rbp)\n", posicao_pilha);
					goto fim;
				}

				vi_ig_pi:
				r = sscanf(linhas[i], "vi%d = pi%d", &indice, &param1);
				if(r == 2) {

					param1 = descobre_reg_param(param1);
					posicao_pilha = pilha_enderecos[indice];

					switch (param1) {
						case 1:
							printf("\n\tmovl %%edi, %d(%%rbp)\n", posicao_pilha);
							break;

						case 2:
							printf("\n\tmovl %%esi, %d(%%rbp)\n", posicao_pilha);
							break;

						case 3:
							printf("\n\tmovl %%edx, %d(%%rbp)\n", posicao_pilha);
							break;
					}
					goto fim;
				}

				r = sscanf(linhas[i], "if %ci%d", &c1, &indice);
				if(r == 2){
					printf("\nif%d:\n", count_if);
					if(c1 == 'c') {
						printf("\tmovl $%d, %%r8d\n", indice);
						printf("\tcmpl $0, %%r8d\n");
						printf("\tje fim_if%d\n", count_if);
						goto body_if;
					}

					if(c1 == 'p') {
						param1 = descobre_reg_param(indice);

						if(param1 == 1)
							printf("\tcmpl $0, %%edi\n");

						if(param1 == 2)
							printf("\tcmpl $0, %%esi\n");

						if(param1 == 3)
							printf("\tcmpl $0, %%edx\n");

						printf("\tje fim_if%d\n", count_if);

						goto body_if;
					}

					if(c1 == 'v') {
						posicao_pilha = pilha_enderecos[indice];
						printf("\tleaq %d(%%rbp), %%r8\n", posicao_pilha);
						printf("\tcmpl $0, (%%r8)\n");
						printf("\tje fim_if%d\n", count_if);
						goto body_if;
					}

					body_if:
					i++;
					int ret_if = if_cases(linhas[i]);

					switch (ret_if) {
						case 1:
							goto vi_ci_mul_ci;

						case 2:
							goto vi_ci_som_ci;

						case 3:
							goto vi_vi_mul_vi;

						case 4:
							goto vi_vi_soma_vi;

						case 5:
							goto vi_vi_mul_ci;

						case 6:
							goto vi_vi_soma_ci;

						case 7:
							goto vi_ci_mul_vi;

						case 8:
							goto vi_ci_som_vi;

						case 9:
							goto vi_vi;

						case 10:
							goto vi_ci;

						case 11:
							goto set_const;

						case 12:
							goto get_vi;

						case 13:
							goto ret_vi;

						case 14:
							goto ret_ci;

						case 15:
							goto ret_pi;

						case 16:
							goto vi_pi_som_pi;

						case 17:
							goto vi_pi_mul_pi;

						case 18:
							goto vi_pi_som_vi;

						case 19:
							goto vi_pi_mul_vi;

						case 20:
							goto vi_pi_som_ci;

						case 21:
							goto vi_pi_mul_ci;

						case 22:
							goto vi_ci_som_pi;

						case 23:
							goto vi_ci_mul_pi;

						case 24:
							goto vi_vi_som_pi;

						case 25:
							goto vi_vi_mul_pi;

						case 26:
							goto vi_ig_pi;

						case 27:
							goto set_vi;

						case 28:
							goto call;
					}

					goto fim;
				}//if

				//vi = vi
				vi_vi:
				r = sscanf(linhas[i], "vi%d = vi%d", &indice1, &indice2);
				if(r == 2) {
					posicao_pilha = pilha_enderecos[indice2];
					printf("\n\t#vi%d = vi%d\n", indice1, indice2);
					printf("\tleaq %d(%%rbp), %%r9\n", posicao_pilha);
					printf("\tmovl (%%r9), %%r10d\n");
					posicao_pilha = pilha_enderecos[indice1];
					printf("\tmovl %%r10d, %d(%%rbp)\n", posicao_pilha);
				}

				//vi = ci
				vi_ci:
				r = sscanf(linhas[i], "vi%d = ci%d", &indice, &constante1);
				if(r == 2) {

					posicao_pilha = pilha_enderecos[indice];
					printf("\n\t#vi%d = %d\n", indice, constante1);
					printf("\tmovl $%d, %d(%%rbp)\n", constante1, posicao_pilha);

				}

				call:
				r = sscanf(linhas[i], "vi%d  = call %d", &indice, &nF);
				if(r == 2) {
					switch (flag_salvamento_regs) {
						case 0:
							printf("\n");
							chamada_funcao(linhas[i], pilha_enderecos, qtd_declaracoes);
							goto fim;

						case 8:
							printf("\n\t#salva reg\n");
							printf("\tmovq %%rdi, -8(%%rbp)\n");
							chamada_funcao(linhas[i], pilha_enderecos, qtd_declaracoes);
							printf("\n\t#recupera reg\n");
							printf("\tmovq -8(%%rbp), %%rdi\n");
							goto fim;

						case 16:
							printf("\n\t#salva regs\n");
							printf("\tmovq %%rdi, -8(%%rbp)\n");
							printf("\tmovq %%rsi, -16(%%rbp)\n");
							chamada_funcao(linhas[i], pilha_enderecos, qtd_declaracoes);
							printf("\n\t#recupera regs\n");
							printf("\tmovq -8(%%rbp), %%rdi\n");
							printf("\tmovq -16(%%rbp), %%rsi\n");
							goto fim;

						case 24:
							printf("\n\t#salva regs\n");
							printf("\tmovq %%rdi, -8(%%rbp)\n");
							printf("\tmovq %%rsi, -16(%%rbp)\n");
							printf("\tmovq %%rdx, -24(%%rbp)\n");
							chamada_funcao(linhas[i], pilha_enderecos, qtd_declaracoes);
							printf("\n\t#recupera regs\n");
							printf("\tmovq -8(%%rbp), %%rdi\n");
							printf("\tmovq -16(%%rbp), %%rsi\n");
							printf("\tmovq -24(%%rbp), %%rdx\n");
							goto fim;
					}
				}

				fim:
				sscanf(linhas[i], "%s", palavra);
				i++;

			}//while end
		}//if function

		sscanf(linhas[i], "%s", palavra);

	}//while externo
} //tradutor

// Remove o '\n' do fim da linha (funcao retirada do arquivo "leitor.c")
void remove_newline(char *line) {
  int i = strlen(line) - 1;
  while (i >= 0 && line[i] == '\n')
    line[i--] = '\0';
}

/*Verifica pi<num>. Retorna 1 para %edi; 2 para %esi; 3 para %edx*/
int descobre_reg_param(int param) {
	switch (param) {
		case 1:
			return 1;

		case 2:
			return 2;

		case 3:
			return 3;
	}
}

/*Verifica quantos paramentos cada funcao possui e entao retorna a qtd necessaria para que
seja possivel salva-los na pilha, caso necessario
Se 1 param, tamanho 8; se 2, 16; se 3, 24
*/
int estabelece_espaco_pilha_para_params(char* linha) {

	int r;
	char c1;
	int id1;

	r = 0;
	r = sscanf(linha, "function %c%c%d, %c%c%d, %c%c%d", &c1, &c1, &id1, &c1, &c1, &id1, &c1, &c1, &id1);
	if(r == 0 || r == 1 || r < 0)
		return 0;

	if(r == 3)
		return 8;

	if(r == 6)
		return 16;

	if(r == 9)
		return 24;

}//estabelece_espaco_pilha_para_params

/*Cada retorno correponde a uma possivel traducao presente na funcao tradutor*/
int if_cases(char linha[256]) {

	int size;
	int indice, indice1, indice2, indice_v, constante1, constante2;
	int r, retorno, nF;
	char op;

	r = sscanf(linha, "vi%d = ci%d %c ci%d", &indice, &constante1, &op, &constante2);
	if(r == 4 && op == '*'){
		return 1;
	}

	r = sscanf(linha, "vi%d = ci%d %c ci%d", &indice, &constante1, &op, &constante2);
	if(r == 4 && op == '+'){
		return 2;
	}

	//vi = vi op vi
	r = sscanf(linha, "vi%d = vi%d %c vi%d", &indice, &constante1, &op, &constante2);
	if(r == 4 && op == '*'){
		return 3;
	}

	r = sscanf(linha, "vi%d = vi%d %c vi%d", &indice, &indice1, &op, &indice2);
	if(r == 4 && op == '+'){
		return 4;
	}

	//vi = vi op ci
	r = sscanf(linha, "vi%d = vi%d %c ci%d", &indice, &indice1, &op, &constante1);
	if(r == 4 && op == '*'){
		return 5;
	}

	r = sscanf(linha, "vi%d = vi%d %c ci%d", &indice, &indice1, &op, &constante1);
	if(r == 4 && op == '+'){
		return 6;
	}

	//vi = ci op vi
	r = sscanf(linha, "vi%d = ci%d %c vi%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '*'){
		return 7;
	}

	r = sscanf(linha, "vi%d = ci%d %c vi%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '+'){
		return 8;
	}

	//vi = vi
	r = sscanf(linha, "vi%d = vi%d", &indice1, &indice2);
	if(r == 2) {
		return 9;
	}

	//vi = ci
	r = sscanf(linha, "vi%d = ci%d", &indice, &constante1);
	if(r == 2) {
		return 10;
	}

	//set va index ci with ci
	r = sscanf(linha, "set va%d index ci%d with ci%d", &indice, &indice_v, &constante1);
	if(r == 3) {
		return 11;
	}

	//get va index ci to vi
	r = sscanf(linha, "get va%d index ci%d to vi%d", &indice_v, &constante1, &indice);
	if(r == 3) {
		return 12;
	}

	//return vi
	r = sscanf(linha, "return vi%d", &retorno);
	if(r == 1) {
		return 13;
	}

	//return ci
	r = sscanf(linha, "return ci%d", &retorno);
	if(r == 1) {
		return 14;
	}

	//return pi
	r = sscanf(linha, "return pi%d", &retorno);
	if(r == 1) {
		return 15;
	}

	r = sscanf(linha, "vi%d = pi%d %c pi%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '+'){
		return 16;
	}

	r = sscanf(linha, "vi%d = pi%d %c pi%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '*'){
		return 17;
	}

	r = sscanf(linha, "vi%d = pi%d %c vi%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '+'){
		return 18;
	}

	r = sscanf(linha, "vi%d = pi%d %c vi%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '*'){
		return 19;
	}

	r = sscanf(linha, "vi%d = pi%d %c ci%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '+'){
		return 20;
	}

	r = sscanf(linha, "vi%d = pi%d %c ci%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '*'){
		return 21;
	}

	r = sscanf(linha, "vi%d = ci%d %c pi%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '+'){
		return 22;
	}

	r = sscanf(linha, "vi%d = ci%d %c pi%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '*'){
		return 23;
	}

	r = sscanf(linha, "vi%d = vi%d %c pi%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '+'){
		return 24;
	}

	r = sscanf(linha, "vi%d = vi%d %c pi%d", &indice, &constante1, &op, &indice1);
	if(r == 4 && op == '*'){
		return 25;
	}

	r = sscanf(linha, "vi%d = pi%d", &indice, &constante1);
	if(r == 2){
		return 26;
	}

	r = sscanf(linha, "set va%d index ci%d with vi%d", &indice, &indice_v, &constante1);
	if(r == 3) {
		return 27;
	}

	r = sscanf(linha, "vi%d  = call %d", &indice, &nF);
	if(r == 2)
		return 28;
}

/*traduz chamadas de funcao*/
void chamada_funcao(char linhas[256], int pilha[], int tam_pilha){

	int r, i;
	int p1, p2, p3;
	int nF, indice;
	char c1, c2, c3, c4, c5, c6;
	char param1[3], param2[3], param3[3];
	int posicao;

	r = sscanf(linhas, "vi%d  = call %d %c%c%d %c%c%d %c%c%d", &indice, &nF, &c1, &c2, &p1, &c3, &c4, &p2, &c5, &c6, &p3);

	param1[0] = c1;
	param1[1] = c2;
	param1[2] = '\0';
	param2[0] = c3;
	param2[1] = c4;
	param2[2] = '\0';
	param3[0] = c5;
	param3[1] = c6;
	param3[2] = '\0';

	if(r == 5 || r == 8 || r == 11) {

		if(strcmp(param1, "vi") == 0){
			posicao = pilha[p1];
			printf(	"\n\tmovl %d(%%rbp), %%edi\n", posicao);
		}

		if(strcmp(param1, "va") == 0){
			posicao = pilha[p1];
			printf(	"\n\tleaq %d(%%rbp), %%rdi\n", posicao);
		}

		if(strcmp(param1, "pi") == 0){
			switch (p1) {
				case 1:
					printf("\n\t#valor de pi1 ja esta em edi\n");
					break;
				case 2:
					printf("\n\tmovl %%esi, %%edi\n");
					break;

				case 3:
					printf("\n\tmovl %%edx, %%edi\n");
					break;
			}

		}

		if(strcmp(param1, "pa") == 0){
			posicao = -8;
			printf(	"\tleaq %d(%%rbp), %%rdx\n", posicao);
		}

		if(strcmp(param1, "ci") == 0){
			printf("\n\tmovl $%d, %%edi\n", p1);
		}

		if(r == 8 || r == 11) {
			if(strcmp(param2, "vi") == 0){
				posicao = pilha[p2];
				printf(	"\n\tmovl %d(%%rbp), %%esi\n", posicao);
			}

			if(strcmp(param2, "va") == 0){
				posicao = pilha[p2];
				printf(	"\tleaq %d(%%rbp), %%rsi\n", posicao);

			}

			if(strcmp(param2, "pi") == 0){
				switch (p2) {
					case 1:
						printf("\tmovl %%edi, %%esi\n");
						break;
					case 2:
						printf("\t#valor de pi2 ja esta em esi\n");
						break;

					case 3:
						printf("\tmovl %%edx, %%esi\n");
						break;
				}
			}

			if(strcmp(param2, "pa") == 0){
				posicao = -16;
				printf(	"\tleaq %d(%%rbp), %%rdx\n", posicao);
			}

			if(strcmp(param2, "ci") == 0){
				printf("\tmovl $%d, %%esi\n", p2);
			}

			if(r == 11) {
				if(strcmp(param3, "vi") == 0){
					posicao = pilha[p3];
					printf(	"\n\tmovl %d(%%rbp), %%edx\n", posicao);
				}

				if(strcmp(param3, "va") == 0){
					posicao = pilha[p3];
					printf(	"\tleaq %d(%%rbp), %%rdx\n", posicao);
				}

				if(strcmp(param3, "pi") == 0){
					switch (p3) {
						case 1:
							printf("\tmovl %%edi, %%edx\n");
							break;
						case 2:
							printf("\tmovl %%esi, %%edx\n");
							break;

						case 3:
							printf("\t#valor de pi3 ja esta em edx\n");
							break;
					}
				}

				if(strcmp(param3, "pa") == 0){
					posicao = -24;
					printf(	"\tleaq %d(%%rbp), %%rdx\n", posicao);
				}

				if(strcmp(param3, "ci") == 0){
					printf("\tmovl $%d, %%edx\n", p3);
				}
			}//if3
		}//if2
	} //if1

	printf("\tmovl $0, %%eax\n");
	printf("\tcall function%d\n", nF);
	posicao = pilha[indice];
	printf("\t#vi%d = ret\n", indice);
	printf("\tmovl %%eax, %d(%%rbp)\n", posicao);
}

int conta_qtd_linhas_arq() {
	int qtd = 0;
	FILE *arquivo;
	arquivo = fopen("prog.slp", "r");
	char line[256];

	if (arquivo == NULL)
		return EXIT_FAILURE;

	while(fgets(line, sizeof line, arquivo) != NULL) {
		qtd++;
	}

	fclose(arquivo);
	return qtd;

}
