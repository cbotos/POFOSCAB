# POFOSCAB - Atari Portfolio Smart Cable tools

Tools for talking to an Atari Portfolio over its Smart Cable interface
(`int 0x61`). `PFTD` is a TSR that extends the Portfolio ROM's file
transfer protocol with extra commands (directory listing, mkdir,
delete, rmdir, rename, copy, clock access).

Full docs: <https://petrkr.github.io/POFOSCAB/>

## Quick start

```bash
# Build PFTD.COM to build/PFTD.COM
make pftd

# Run tests against an already-running Smart Cable bridge
pip install pytest   # in a venv
pytest tests/ -v
```
