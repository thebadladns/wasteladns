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
elseif has ('win64')
    let g:config='debug'
    let g:outdir='Debug'
    set makeprg=..\..\build.bat
    let g:target='dx11'
    noremap <leader>bd :let g:target='dx11'<CR>
    noremap <leader>bg :let g:target='gl33'<CR>
    noremap <expr> <leader>bb ":wa\|:make " . g:target . " " . g:config . "<CR>"  " call make and expand the given variables
    noremap <leader>rr :!cd .\..\..\bin\<c-r>=g:outdir<cr> && START /B app-<c-r>=g:target<cr>.exe<CR>
endif
