# prepare_files_to_signed.sh

Guia rapido sobre o script utilizado para preparar imagens para assinatura e envio para a Nagra.

## Visao geral
- Copia imagens UBO de uma pasta origem para uma pasta destino limpa.
- Gera uma versao alinhada em blocos de 16 bytes de cada imagem.
- Realiza a criptografia com GPG para a chave `NASC_signing@nagra.com`.

## Pre-requisitos
- `bash`, `dd`, `stat`, `bc`, `gpg` instalados e no PATH.
- Chave publica `NASC_signing@nagra.com` importada no keyring GPG local.

## Uso
```bash
./prepare_files_to_signed.sh <pasta_origem> <pasta_destino>
```
- `pasta_origem`: local das imagens de entrada (ex.: see_bl.ubo, main.ubo, logo.ubo, upg_onepackage.ubo, dtb.ubo, roothash.ubo, see.ubo).
- `pasta_destino`: pasta que recebera as copias e onde sera criada a pasta alinhada.

## Passos executados
1) Define pastas base e lista de imagens obrigatorias.
2) Limpa e recria `pasta_destino`, copiando todas as imagens originais para ela.
3) Cria a pasta `out_16ByteAlign` dentro do diretorio atual:
   - Recria cada imagem com tamanho alinhado a 16 bytes.
   - Se o tamanho nao for multiplo de 16, preenche o resto com zeros usando `dd`.
4) Entra em `out_16ByteAlign` e cifra cada imagem com GPG usando o destinatario `NASC_signing@nagra.com`.

## Saida esperada
- `<pasta_destino>/`: copias diretas das imagens de entrada.
- `out_16ByteAlign/`: versoes alinhadas e os arquivos cifrados (`*.gpg`).

## Observacoes
- O script usa `set -e`; qualquer falha interrompe a execucao.
- Tenha espaco em disco suficiente para duplicar as imagens.
- Confirme permissao de escrita nas pastas de destino e no diretorio atual.
- Ao término os arquivos devem assinados no site da Nagra

# gpg_decrypt.sh

Script de pos-processamento que recebe os pacotes assinados/cifrados e monta o pacote OTA final para B8.

## Visao geral
- Copia os arquivos `.lbs.pgp` assinados das pastas de SignedFiles para um destino temporario.
- Decifra os binarios obrigatorios, monta `rootfs.squashfs` e recalcula hashes.
- Gera imagem OTA (`*.b8`) compactada em squashfs e depois cifra com AES-xts via dm-crypt/losetup.
- Embute versao minima e versao atual no arquivo final e gera o transporte TS via `mkts`.

## Uso
```bash
./gpg_decrypt.sh <pasta_origem> <pasta_destino> <versao_minima> <versao_atual>
```
- `pasta_origem`: raiz contendo SignedFiles/* com os `.lbs.pgp` assinados.
- `pasta_destino`: pasta de trabalho para copiar e decifrar arquivos.
- `versao_minima`: versao minima aceita (ex.: 1.2).
- `versao_atual`: versao do pacote gerado (ex.: 3.4).

## Passos executados
1) Copia cada arquivo `*.lbs.pgp` listado no array `images` para `pasta_destino` e decifra em `B8_signed_decrypt_<versao_atual>`.
2) Copia `rootfs.squashfs` esperado em `B8_*_images/images_upg/` para a mesma pasta decifrada.
3) Monta `Update_B8` com `main.ubo`, `roothash.ubo`, `rootfs.squashfs`, gera `sha256sum.txt` e empacota em squashfs (`*.b8`).
4) Cria loop device, cifra o pacote OTA com chave AES-xts fixa e remove o original.
5) Insere bytes da versao atual no inicio do arquivo, renomeia para `*.b8` cifrado e chama `mkts` para gerar o TS final com PID/PROD_ID fixos.
6) Limpa temporarios intermediarios (`*.org`).

## Saidas esperadas
- `B8_signed_decrypt_<versao_atual>/`: binarios decifrados e `rootfs.squashfs` copiados.
- `Update_B8/`: pasta intermediaria com conteudo para o squashfs e `sha256sum.txt`.
- `B8_<data>_update_enc_v<versao_atual>.b8`: pacote OTA cifrado usado na etapa `mkts`.
- `<data>_ota_century_b8_v<versao_atual>_pid_6041_ClaroKu.ts`: arquivo TS final.

## Observacoes
- Requer `gpg`, `mksquashfs`, `losetup`, `dmsetup`, `dd`, `sha256sum`, `fallocate` e permissao de sudo para mapear loop device.
- Chave AES encontra-se hardcoded no script; revisar politicas de segredo antes de distribuir.
- Pastas e nomes de arquivos sao assumidos (SignedFiles/*, B8_*_images/...); ajuste se a estrutura mudar.
- O script nao remove `pasta_destino` nem `Update_B8` ao final (linhas comentadas); limpe manualmente se necessario.
