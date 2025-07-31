# === CONFIGURATION ===

# Configuration de base MarsDev
export GDK ?= /opt/toolchains/mars/m68k-elf

# Variables pour le projet
PROJECT_NAME = urban_thunder
PYTHON_ENV = .venv/bin/python

# === CIBLES PERSONNALISÉES ===

# Génération automatique des images de remplacement
generate-assets:
	@echo "Génération des assets de remplacement..."
	$(PYTHON_ENV) create_simple_images.py

# Génération des ressources SGDK
resources.h resources.rs: resources.res generate-assets
	@echo "Compilation des ressources SGDK..."
	java -jar $(GDK)/bin/rescomp.jar resources.res resources.rs -header resources.h -dep resources.d
	@if [ ! -d inc ]; then mkdir -p inc; fi
	cp resources.h inc/

# Nettoyage des ressources
clean-resources:
	rm -f resources.h resources.rs resources.d
	rm -f inc/resources.h

# Build avec génération automatique des ressources
build: resources.h resources.rs
	$(MAKE) -f $(GDK)/makefile.gen

# === CIBLES DE TEST ===

# Test avec émulateur (configurable)
EMULATOR ?= retroarch
EMU_CORE ?= /usr/lib/libretro/genesis_plus_gx_libretro.so

test: build
	@echo "Lancement du test avec $(EMULATOR)..."
	@if [ -f out.bin ]; then \
		$(EMULATOR) -L $(EMU_CORE) out.bin 2>/dev/null || \
		echo "Émulateur non trouvé, utilisez: make test-file"; \
	else \
		echo "Erreur: out.bin non trouvé. Lancez 'make build' d'abord."; \
	fi

# Test simple (affiche juste le fichier)
test-file: build
	@echo "ROM générée:"
	@ls -la out.bin 2>/dev/null || echo "Aucune ROM trouvée"

# Test alternatif avec gens
test-gens: build
	@if command -v gens >/dev/null 2>&1; then \
		gens out.bin; \
	else \
		echo "Gens non installé"; \
	fi

# === CIBLES DE VALIDATION ===

# Validation de la ROM
validate: build
	@echo "=== VALIDATION DE LA ROM ==="
	@if [ -f out.bin ]; then \
		echo -n "Taille de la ROM: "; \
		stat -c%s out.bin | awk '{printf "%.2f KB\n", $$1/1024}'; \
		echo -n "Taille max Mega Drive: 4096 KB - "; \
		if [ $$(stat -c%s out.bin) -lt 4194304 ]; then \
			echo "✓ OK"; \
		else \
			echo "✗ TROP GROSSE"; \
		fi; \
	else \
		echo "✗ ROM non trouvée"; \
	fi

# === CIBLES DE DÉVELOPPEMENT ===

# Génération rapide des assets et build
quick: generate-assets build

# Surveillance continue (nécessite inotify-tools)
watch:
	@echo "Mode surveillance activé. Ctrl+C pour arrêter."
	@echo "Surveillance des fichiers: src/, res/, resources.res, create_simple_images.py"
	@while true; do \
		inotifywait -e modify -r src/ res/ resources.res create_simple_images.py 2>/dev/null || true; \
		echo "Changement détecté, recompilation..."; \
		$(MAKE) quick && echo "✓ Compilation réussie" || echo "✗ Erreur de compilation"; \
		sleep 1; \
	done

# === CIBLES D'ANALYSE ===

# Métriques du code
metrics:
	@echo "=== MÉTRIQUES DU CODE ==="
	@echo -n "Lignes de code C: "
	@find src -name "*.c" -exec cat {} \; 2>/dev/null | wc -l || echo "0"
	@echo -n "Lignes de code ASM: "
	@find src -name "*.s" -exec cat {} \; 2>/dev/null | wc -l || echo "0"
	@echo -n "Lignes de headers: "
	@find src inc -name "*.h" -exec cat {} \; 2>/dev/null | wc -l || echo "0"
	@echo -n "Nombre de fonctions C: "
	@grep -r "^[a-zA-Z_][a-zA-Z0-9_]*.*(" src/*.c 2>/dev/null | wc -l || echo "0"

# Analyse de la taille du code
code-size: build
	@echo "=== ANALYSE DE TAILLE ==="
	@if [ -f out.elf ]; then \
		$(GDK)/bin/m68k-elf-size out.elf 2>/dev/null || echo "Impossible d'analyser out.elf"; \
	else \
		echo "out.elf non trouvé"; \
	fi

# === CIBLES DE NETTOYAGE ===

# Nettoyage complet
clean: clean-resources
	$(MAKE) -f $(GDK)/makefile.gen clean
	rm -f *.bin *.elf *.map
	rm -rf out/

# Nettoyage des fichiers Python
clean-python:
	find . -name "*.pyc" -delete
	find . -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true

# Nettoyage complet incluant les assets générés
clean-all: clean clean-python
	rm -f res/road_simple.png res/grass_simple.png res/sky_simple.png res/simple_palette.png

# === CIBLES D'AIDE ===

help:
	@echo "=== URBAN THUNDER - SYSTÈME DE BUILD ==="
	@echo ""
	@echo "Cibles principales:"
	@echo "  build            - Compile le projet avec génération auto des ressources"
	@echo "  quick            - Génération rapide des assets et build"
	@echo "  generate-assets  - Génère uniquement les images de remplacement"
	@echo "  clean           - Nettoie les fichiers générés"
	@echo "  clean-all       - Nettoyage complet incluant les assets"
	@echo ""
	@echo "Tests:"
	@echo "  test            - Lance avec l'émulateur par défaut"
	@echo "  test-file       - Affiche la ROM générée"
	@echo "  test-gens       - Lance avec Gens (si installé)"
	@echo "  validate        - Valide la ROM générée"
	@echo ""
	@echo "Développement:"
	@echo "  watch           - Mode surveillance (recompile auto)"
	@echo "  metrics         - Affiche les métriques du code"
	@echo "  code-size       - Analyse de la taille du code"
	@echo ""
	@echo "Variables d'environnement:"
	@echo "  EMULATOR        - Émulateur à utiliser (défaut: retroarch)"
	@echo "  GDK            - Chemin vers MarsDev (défaut: /opt/toolchains/mars/m68k-elf)"
	@echo ""
	@echo "Exemples:"
	@echo "  make build test          - Build et test"
	@echo "  make EMULATOR=gens test  - Test avec Gens"
	@echo "  make watch              - Surveillance continue"

# === INTÉGRATION MARSDEV ===

# Inclusion du système MarsDev (doit être en dernier)
include $(GDK)/makefile.gen

# === RÈGLES SPÉCIALES ===

# Cibles qui ne correspondent pas à des fichiers
.PHONY: generate-assets clean-resources build test test-file test-gens validate quick watch metrics code-size clean clean-python clean-all help

# Évite la suppression des fichiers intermédiaires
.PRECIOUS: resources.h resources.rs
