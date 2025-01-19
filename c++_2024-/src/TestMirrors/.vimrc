" load the conf for this project by adding these 3 lines to your vim config file, then loading vim in this folder
"if filereadable(".vimrc")
"    so .vimrc
"endif
:echo "Loading vimrc at local directory: " . expand('%:p')

" tab = 4 spaces
set tabstop=4
set expandtab
set shiftwidth=4
" build/run commands
noremap <leader>bcd :let [g:config,g:outdir]=['debug','Debug']<CR>
noremap <leader>bcr :let [g:config,g:outdir]=['release','Release']<CR>
if has('mac')
    let g:config='release'
    let g:outdir='Release'
    set makeprg=sh\ ../../build.sh
    noremap <expr> <leader>bb ":wa\|:make " . g:config . "<CR>"  " call make and expand the given variables
    noremap <leader>rr :!cd ./../../bin/<c-r>=g:outdir<cr> && ./app-macos &<CR>
    " old scripts, for reference:
    "let &makeprg='xcodebuild -project ../../wasteladns.xcodeproj -scheme app-macos'
    "   let &makeprg='clang++ -x objective-c++ -std=gnu++14 -fobjc-arc -D__MACOS=1 -D__GL33=1 -framework Cocoa -framework IOKit main.cpp -o ../../bin/Debug/app-macos'
    "   noremap <leader>rr :!cd ./../../bin/Debug && ./app-macos &<CR>
    "" save all buffers, then compile
    "noremap <leader>bb :wa\|:make<CR>
elseif has ('win64')
    let g:config='debug'
    let g:outdir='Debug'
    set makeprg=..\..\build.bat
    let g:target='dx11'
    noremap <leader>bd :let g:target='dx11'<CR>
    noremap <leader>bg :let g:target='gl33'<CR>
    noremap <expr> <leader>bb ":wa\|:make " . g:target . " " . g:config . "<CR>"  " call make and expand the given variables
    "noremap <leader>bb :wa\|:make <c-r>=g:target<cr> <c-r>=g:config<cr><CR>"  " call make and expand the given variables
    noremap <leader>rr :!cd .\..\..\bin\<c-r>=g:outdir<cr> && START /B app-<c-r>=g:target<cr>.exe<CR>
    
    " old scripts, for reference:
    " build (only works if cmd has the rigth env variables)
    "   (use bats in C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Visual Studio 2022\Visual Studio Tools\VC)
    "set makeprg=msbuild\ ..\..\wasteladns.sln\ /nologo\ /v:q\ /property:GenerateFullPaths=true
    "let &makeprg='cl /Fe"..\..\bin\Debug\app-dx11.exe" /Fo..\..\app-dx11.dir\Debug\ main.cpp "user32.lib" /D "__WIN64=1" /D "__DX11=1"'
    "noremap <leader>bd :set makeprg=msbuild\ ..\..\app-dx11.vcxproj\ /nologo\ /v:q\ /property:GenerateFullPaths=true<CR>
    "noremap <leader>bg :set makeprg=msbuild\ ..\..\app-gl33.vcxproj\ /nologo\ /v:q\ /property:GenerateFullPaths=true<CR>
    "noremap <leader>bd :let &makeprg='cl /Fe"..\..\bin\Debug\app-dx11.exe" /Fo..\..\app-dx11.dir\Debug\ main.cpp "user32.lib" /D "__WIN64=1" /D "__DX11=1"'<CR>
    "noremap <leader>bg :let &makeprg='cl /Fe"..\..\bin\Debug\app-dx11.exe" /Fo..\..\app-dx11.dir\Debug\ main.cpp "user32.lib" "gdi32.lib" /D "__WIN64=1" /D "__GL33=1"'<CR>
endif
