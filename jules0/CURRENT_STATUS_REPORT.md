# Huidige Statusrapport van de Point Cloud Processing Pipeline

Dit document analyseert de staat van de C++/CUDA codebase, met een specifieke focus op de `doOptimizeFrame2` en `doOptimizeFrame3` pipelines. De `doOptimizeFrame1` pipeline en de `VoxelManager` zijn buiten beschouwing gelaten, zoals gevraagd.

## 1. Kern Focus: `doOptimizeFrame2` en `doOptimizeFrame3`

De analyse bevestigt dat de primaire logica voor de oudere, niet-voxel-gebaseerde pipeline zich in `doOptimizeFrame2` en `doOptimizeFrame3` bevindt. De meest complete implementatie van deze oudere workflow is te vinden in `legacy/src/h_do_optimize_frame3.cpp`.

## 2. Analyse van Buisdetectie (`h_buis_vote_bruteforce`)

**Vraag:** Welke low-level kernels zijn momenteel geïmplementeerd om de steunpunten (**M**) en de straal (**R**) van buizen te bepalen in de `h_buis_vote_bruteforce` pipeline?

**Antwoord:**
Er zijn **geen expliciete low-level kernels** direct zichtbaar in de codebase voor het berekenen van **M** en **R** binnen de `h_buis_vote_bruteforce` functie. De analyse wijst op het volgende:

*   **Hoog-niveau Orchestratie:** `h_buis_vote_bruteforce` is een hoog-niveau host-functie die, net als de `h_vlak_vote_*` functies, waarschijnlijk een stem-procedure initieert om een kandidaat-as voor een buis te vinden. De definitie van deze functie is niet aanwezig in de codebase, wat suggereert dat deze mogelijk deel uitmaakt van een externe, voorgecompileerde bibliotheek.
*   **Geen "Slab & EMA-regressie":** Er is geen implementatie gevonden die overeenkomt met de "Slab & EMA-regressie" methode zoals beschreven in `design.md`.
*   **Verdeelde Verantwoordelijkheid:** De `design.md` suggereert een proces in meerdere stappen:
    1.  Eerst wordt een ruwe as-richting bepaald (de taak van `h_buis_vote_bruteforce`).
    2.  Vervolgens, na de rotatie naar het ideale frame, worden de precieze parameters (**M** en **R**) berekend. De functie `h_fine_slabs_cyl_label` in `legacy/src/h_do_optimize_frame3.cpp` wijst in deze richting, maar de implementatie ervan is eveneens niet aanwezig.

**Conclusie:** De bepaling van **M** en **R** is niet de verantwoordelijkheid van `h_buis_vote_bruteforce`. Deze functie focust zich uitsluitend op het vinden van een *richting*. De daadwerkelijke parameter-extractie kernels zijn niet geïmplementeerd of niet aanwezig in de geanalyseerde code.

## 3. Rol van de `VoxelManager`

**Vraag:** Wordt de `VoxelManager` op dit moment gebruikt om de $30 \times 30 \times 30$ mm clusters te definiëren, of is dat een topologische beslissing die in de Host-code wordt genomen?

**Antwoord:**
De `VoxelManager` wordt **niet gebruikt** in de `doOptimizeFrame2` en `doOptimizeFrame3` pipelines.

*   **Geen Aanroepen:** Een zoekopdracht in de `legacy/src` directory bevestigt dat er geen enkele verwijzing naar of aanroep van de `VoxelManager` is in de oudere pipelines.
*   **Topologische Beslissing op de Host:** De `design.md` beschrijft de 30mm-clusters (of "voxels" in die context) als een topologische aggregatie-eenheid die wordt gebruikt *na* de rotatie naar het ideale frame. Dit is bedoeld voor de `PipeFittingDetector` op de CPU. De `doOptimizeFrame3` pipeline voert wel een Morton-sortering uit—een voorbereidende stap voor efficiënte ruimtelijke clustering—maar implementeert de uiteindelijke 30mm-clustering niet.

**Conclusie:** De definitie van de 30x30x30 mm clusters is een topologische beslissing op de host die in de `doOptimizeFrame2`/`3` pipelines **nog niet is geïmplementeerd**.
