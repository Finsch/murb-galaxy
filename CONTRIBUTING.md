
## Guide de contribution - Projet MUrB

### Workflow Git
Nous utilisons Git Flow avec les conventions suivantes :

#### Types de branches
- `main` : Code stable et validé
- `develop` : Branche d'intégration
- `feature/*` : Nouvelles fonctionnalités/implémentations

#### Convention de nommage des branches
feature/sequential
feature/simd
feature/multi-thread
feature/gpu
feature/hybrid

#### Messages de commit
Utiliser le format conventionnel :

feat: implémentation SIMD avec MIPP
fix: correction calcul accélération
docs: mise à jour README
test: ajout tests OpenMP

### Exemple concret avec Bob.

#### Situation : Bob travaille sur l'optimisation séquentielle

##### Etape 1 :  Bob prépare son environnement
```
# 1. Clone le dépôt
git clone https://github.com/Finsch/murb-galaxy.git
cd murb-galaxy

# 2. Récupère la dernière version
git checkout develop
git pull origin develop

# 3. Crée SA branche de feature
git checkout -b feature/sequential

# 4. Pousse la branche sur GitHub
git push -u origin feature/sequential
```

##### Étape 2 : Bob développe localement
```
# Travaille sur son implémentation...
# Modifie src/murb/implem/SimulationNBodyOptim.{hpp,cpp}
# Modifie main.cpp pour ajouter le tag cpu+optim

# Commite régulièrement
git add src/murb/implem/SimulationNBodyOptim.hpp
git commit -m "feat: implémentation basique de l'optimisation séquentielle"

git add src/murb/implem/SimulationNBodyOptim.cpp
git commit -m "feat: algorithme optimisé avec réduction des calculs"

git add src/murb/main.cpp
git commit -m "feat: ajout du tag cpu+optim dans main.cpp"

# Pousse ses changements
git push origin feature/sequential
```

##### Étape 3 : Bob crée une Pull Request (PR)

###### Sur GitHub :
1. Va sur https://github.com/Finsch/murb-galaxy
2. Clique sur "Pull requests" → "New pull request"
3. Configure :
    - base: `develop` ← Où on veut merger
    - compare: `feature/sequential` ← Ce qu'on veut merger
4. Remplit le formulaire :
    ```
    Titre: [FEAT] Implémentation de l'optimisation séquentielle

    Description:
    ## Changements
    - Ajout de SimulationNBodyOptim (25% plus rapide que naive)
    - Réduction de la complexité algorithmique
    - Ajout du tag cpu+optim

    ## Tests effectués
    - [x] ./bin/murb-test : tous les tests passent
    - [x] Validation avec n=1000, i=1000 : FPS amélioré de 15 à 22
    - [x] Comparaison avec golden model : résultats identiques

    ## Performance
    Avant (naive): 15 FPS
    Après (optim): 22 FPS (+46%)

    ## Commandes de test
    ./bin/murb -n 5000 -i 1000 --nv --im cpu+naive
    ./bin/murb -n 5000 -i 1000 --nv --im cpu+optim
    ```
5. Assign 2 reviewers (les autres membres de l'équipe)
6. Clique "Create pull request"

##### Étape 5 : Revue de code
- Alice et Charlie reçoivent une notification
- Ils examinent le code :
    - Vérifient la qualité
    - Testent localement si besoin
    - Donnent leur approbation ("Approve") ou commentaires

##### Étape 6 : Merge de la feature
Une fois au moins 1 approbation obtenue :

```
# Option 1 : Merge via GitHub (recommandé)
# - Clique "Merge pull request" sur GitHub
# - Choisir "Squash and merge" pour un historique propre

# Option 2 : Merge manuel
git checkout develop
git pull origin develop
git merge --squash feature/sequential-optim
git commit -m "feat: implémentation de l'optimisation séquentielle"
git push origin develop
```

##### Étape 7 : Nettoyage
```
# Supprime la branche locale
git branch -d feature/sequential-optim

# Supprime la branche sur GitHub
git push origin --delete feature/sequential-optim
```

---

##### When you come back. (bonnes pratiques)
```
# BON : Branche courte et focalisée
git checkout -b feature/one-specific-thing

# BON : Pull régulier de develop
git checkout feature/sequential
git pull origin develop  # Récupère les changements des autres

# Poussez régulièrement
git push origin feature/your-branch
```

En cas de conflit :
```
git checkout feature/your-branch
git pull origin develop
# Résoudre les conflits...
git add .
git commit -m "fix: résolution conflits avec develop"
```

----

### quick access.

git checkout develop
git pull origin develop

git checkout -b feature/sequential

git add .
git status
git commit -m "feat: implémentation de l'optimisation séquentielle"
git push origin feature/sequential
