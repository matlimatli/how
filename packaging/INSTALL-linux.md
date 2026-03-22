# Installing how on Linux

Extract the archive and copy the files:

```sh
sudo cp bin/how /usr/local/bin/
sudo ln -sf /usr/local/bin/how /usr/local/bin/what
sudo ln -sf /usr/local/bin/how /usr/local/bin/when
sudo ln -sf /usr/local/bin/how /usr/local/bin/why
sudo mkdir -p /usr/local/share/man/man1
sudo cp share/man/man1/how.1 /usr/local/share/man/man1/
```

Requires libcurl at runtime (`libcurl4` on Debian/Ubuntu, `libcurl` on Fedora/RHEL).

To uninstall:

```sh
sudo rm -f /usr/local/bin/how /usr/local/bin/what /usr/local/bin/when /usr/local/bin/why
sudo rm -f /usr/local/share/man/man1/how.1
```
