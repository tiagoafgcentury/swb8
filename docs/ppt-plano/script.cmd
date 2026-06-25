@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

echo [ppt-plano] Gerando site MkDocs a partir de docs\plano...

where mkdocs >nul 2>nul
if %ERRORLEVEL%==0 (
    mkdocs build --clean --config-file mkdocs.yml
    goto :done
)

python -m mkdocs --version >nul 2>nul
if %ERRORLEVEL%==0 (
    python -m mkdocs build --clean --config-file mkdocs.yml
    goto :done
)

py -m mkdocs --version >nul 2>nul
if %ERRORLEVEL%==0 (
    py -m mkdocs build --clean --config-file mkdocs.yml
    goto :done
)

echo.
echo [ERRO] MkDocs nao encontrado.
echo Instale o MkDocs e o tema Material antes de executar este script:
echo.
echo     pip install mkdocs mkdocs-material
echo.
exit /b 1

:done
if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERRO] Falha ao gerar o site MkDocs.
    exit /b %ERRORLEVEL%
)

echo.
echo [OK] Portal gerado em: %SCRIPT_DIR%site
echo [OK] Pagina inicial: %SCRIPT_DIR%site\index.html
echo.
endlocal
