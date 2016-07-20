find . -type d \( -iname "moc" -o -iname "ui_header" -o -iname "obj" \) -print0 | xargs -0 rm -rv
