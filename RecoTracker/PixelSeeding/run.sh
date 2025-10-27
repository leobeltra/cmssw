LOCALPATH='/cms-hlt-nfs/data/relValTTbar_200PU/'
echo "Input source: |${LOCALPATH}|"
LOCALFILES=$(ls -1 ${LOCALPATH})
ALL_FILES=""
for f in ${LOCALFILES[@]}; do
    ALL_FILES+="file:${LOCALPATH}/${f},"
done
# Remove the last character

ALL_FILES="${ALL_FILES%?}"
echo "Discovered files: $ALL_FILES"
cmsDriver.py Phase2 -s L1P2GT,HLT:75e33_timing \
--processName=HLTX \
--conditions auto:phase2_realistic_T33 \
--geometry ExtendedRun4D110 \
--era Phase2C17I13M9 \
--customise SLHCUpgradeSimulations/Configuration/aging.customise_aging_1000 \
--eventcontent FEVTDEBUGHLT \
--filein=$ALL_FILES \
--procModifiers alpaka \
--mc \
--inputCommands="keep *, drop *_hlt*_*_HLT, drop triggerTriggerFilterObjectWithRefs_l1t*_*_HLT" \
-n -1 \
--no_exec \
--output={} \
--python_filename hlt_config.py
