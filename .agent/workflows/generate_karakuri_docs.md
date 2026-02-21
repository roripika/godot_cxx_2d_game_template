---
description: Generate Doxygen HTML docs for the Karakuri C++ SDK
---
Run this workflow to regenerate the Karakuri SDK documentation whenever the C++ headers in `src/karakuri/` are updated.

// turbo-all
1. Generate the documentation using Doxygen:
```bash
doxygen docs/karakuri_sdk/Doxyfile
```

2. The HTML API references are now updated and available at `docs/karakuri_sdk/html/index.html`.
