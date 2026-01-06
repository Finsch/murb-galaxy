#!/bin/bash

# Variables
BRANCH_PUSH="develop"
BRANCH_MERGE_FROM="develop"
BRANCH_MERGE_TO="main"
COMMIT_MSG="docs: mise à jour documentation"
MERGE_MSG="release: dev → main"

echo "=== Menu Git Operations ==="
echo "1. Commit et push sur $BRANCH_PUSH"
echo "2. Merge $BRANCH_MERGE_FROM → $BRANCH_MERGE_TO"
read -p "Choix (1 ou 2): " choice

# cd ../murb-galaxy || { echo "Erreur: dossier murb-galaxy introuvable"; exit 1; }

case $choice in
    1)
        echo "=== Commit et push sur $BRANCH_PUSH ==="
        git checkout $BRANCH_PUSH
        git add .
        git commit -m "$COMMIT_MSG"
        git push origin $BRANCH_PUSH
        ;;
    2)
        echo "=== Merge $BRANCH_MERGE_FROM → $BRANCH_MERGE_TO ==="
        git checkout $BRANCH_MERGE_TO
        git pull origin $BRANCH_MERGE_TO
        git checkout $BRANCH_MERGE_FROM
        git pull origin $BRANCH_MERGE_FROM
        
        git checkout $BRANCH_MERGE_TO
        git merge --no-ff $BRANCH_MERGE_FROM -m "$MERGE_MSG"
        git push origin $BRANCH_MERGE_TO
        ;;
    *)
        echo "Choix invalide. Exit."
        exit 1
        ;;
esac

echo "✅ Opération terminée."

# chmod +x pusherman.sh
# ./pusherman.sh 
# ./scripts/pusherman.sh 

## to be continued ...
## create new branches for versionning v0, v1, etc.