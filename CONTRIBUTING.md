
# Guide de contribution - Projet MUrB

## 🏁 Pour commencer
1. Cloner le dépôt : `git clone https://github.com/Finsch/murb-galaxy.git`
2. S'assurer d'avoir les submodules : `git submodule update --init --recursive`
3. Créer une branche : `git checkout -b feature/nom-feature`

## 🔄 Workflow Git
Nous utilisons Git Flow avec les conventions suivantes :

### Types de branches
- `main` : Code stable et validé
- `develop` : Branche d'intégration
- `feature/*` : Nouvelles fonctionnalités/implémentations

### Convention de nommage des branches
feature/optim-sequentielles
feature/simd-avx
feature/omp-parallel
bugfix/correction-memory-leak
test/validation-gpu

### Messages de commit
Utiliser le format conventionnel :

feat: implémentation SIMD avec MIPP
fix: correction calcul accélération
docs: mise à jour README
test: ajout tests OpenMP
refactor: optimisation mémoire


---


git add .
git status
git commit -m "feat: ajout du projet MUrB original"
git push origin main