function []=exemplar_vowelspace()

% h1=figure;
% h2=figure;

%% PARAMETERS

global sBot;
global vSpace;
global sParm;
global cycle_count;

sParm.ibots = 5;
sParm.agerate=.99;
sParm.cycles = 10000;
sParm.exchanges = 10;
sParm.trench = 0;

vSpace.vowels = 3;
vSpace.minfreq=100;
vSpace.maxfreq=1100;
vSpace.JND=1;
vSpace.segments=(vSpace.maxfreq-vSpace.minfreq)/vSpace.JND;
vSpace.var=round(25/vSpace.JND);
vSpace.percept_win=round(110/vSpace.JND);
vSpace.prod_win=round(100/vSpace.JND);

sVowel.iF=zeros(1,vSpace.maxfreq-vSpace.minfreq);
sVowel.sIPA='';

for i=1:sParm.ibots
    for k=1:vSpace.vowels
        sBot(i).sVowel(k).iF=zeros(1,round(vSpace.segments));
    end
end

fLoadBots;

for cycle_count=1:sParm.cycles

    fTalk;
    fAge;
    
    if (mod(cycle_count,10) == 0)
        fPlot; 
    end
    
end

end


%%
function []=fPlot()
global sBot;
global vSpace;
global sParm;

weight=0; meansum=0;
colors={'b','g','y', 'm'};
        
        for i=1:sParm.ibots
        subplot(sParm.ibots,1,i); hold off;

            for j=1:vSpace.vowels

                m(i).v(j).iF = zeros(1,round(vSpace.segments));

               meansum = sum(sBot(i).sVowel(j).iF);
               for k=1:length(sBot(i).sVowel(j).iF)
                     weight = weight + k * sBot(i).sVowel(j).iF(k);
                end;
                if round(meansum) > 0
                    m(i).v(j).iF(round(weight/meansum)) = 2000;
                end
                meansum=0; weight=0;

                subplot(sParm.ibots,1,i); 
                plot(sBot(i).sVowel(j).iF,colors{j}); ylim([0 2000]);
                hold on;
                plot(m(i).v(j).iF, 'r'); ylim([0 2000]);
                hold on;
             
            end
            pause(.0000001);
        end


        
end

function []=fLoadBots()
global sBot;
global vSpace;
global sParm;

% sBot(1).sVowel(1).iF(185)=300;
% sBot(1).sVowel(2).iF(470)=300;
% sBot(2).sVowel(1).iF(135)=100;
% sBot(2).sVowel(2).iF(470)=100;
% 
for i=1:sParm.ibots
%     sBot(1).sVowel(1).sIPA='[i]';
%     sBot(1).sVowel(2).sIPA='[e]';
%     sBot(2).sVowel(1).sIPA='[i]';
%     sBot(2).sVowel(2).sIPA='[e]';

sBot(i).sVowel(1).iF(round(300/vSpace.JND))=1000;
sBot(i).sVowel(2).iF(round(500/vSpace.JND))=1000;
sBot(i).sVowel(3).iF(round(700/vSpace.JND))=1000;
sBot(i).dInfluence=100;
end

sBot(1).sVowel(1).iF(round(100/vSpace.JND))=2000;
sBot(1).sVowel(1).iF(round(300/vSpace.JND))=0;
sBot(1).dInfluence=300;
sBot(2).sVowel(1).iF(round(100/vSpace.JND))=2000;
sBot(2).sVowel(1).iF(round(300/vSpace.JND))=0;
sBot(2).dInfluence=300;

end


%%
function []=fTalk()
global sBot;
global vSpace;
global sParm;

for i=1:sParm.exchanges

    iSpeaker = ceil(sParm.ibots.*rand);
    iVowel = ceil(vSpace.vowels.*rand);
    iHearer = ceil(sParm.ibots.*rand);

    iVowelFreq = fProduceVowel(iSpeaker,iVowel);

    E = vSpace.var*randn;
    iVowelFreq = round(iVowelFreq + E); %+iBias

    if (iVowelFreq < 1); iVowelFreq=1;
    end
    if (iVowelFreq*vSpace.JND + vSpace.minfreq > vSpace.maxfreq);
        iVowelFreq=round(vSpace.segments);
    end %change later

    iNewVowelCat = fCategorizeVowel(iHearer,iVowelFreq);

    if (iNewVowelCat ~= 0)
        sBot(iHearer).sVowel(iNewVowelCat).iF(iVowelFreq) = ...
            sBot(iHearer).sVowel(iNewVowelCat).iF(iVowelFreq) + sBot(iSpeaker).dInfluence;
    end

end

end

%%
function [iVowelFreq]=fProduceVowel(iSpeaker, iVowel)
global sBot;
global vSpace;
global sParm;

exemplar_sum = sum(sBot(iSpeaker).sVowel(iVowel).iF);

target = exemplar_sum*rand;

sum_bins=0;
for i=1:length(sBot(iSpeaker).sVowel(iVowel).iF)
    sum_bins=sum_bins+sBot(iSpeaker).sVowel(iVowel).iF(i);
    if (sum_bins > target); break; end;
end

iVowelFreq=i;

end

%%
function [iNewVowelCat]=fCategorizeVowel(iHearer, iVowelFreq)
global sBot;
global vSpace;
global sParm;

max_weight=0;
weight=0;
iVowelCat = 0;
for i=1:vSpace.vowels

    if (iVowelFreq - vSpace.percept_win < 1);
        lowerbound=1;
    else lowerbound = round(iVowelFreq - vSpace.percept_win);
    end
    if (iVowelFreq + vSpace.percept_win > vSpace.segments); upperbound=round(vSpace.segments);
    else upperbound = round(iVowelFreq + vSpace.percept_win);
    end

    for j = lowerbound:upperbound
        weight=weight+sBot(iHearer).sVowel(i).iF(j);
    end
    if (weight > max_weight)
        iVowelCat = i;
        max_weight = weight;
    end
    weight=0;
end

iNewVowelCat = iVowelCat;

end
%%
function []=fAge()
global sBot;
global vSpace;
global sParm;

for i=1:sParm.ibots
    for j=1:vSpace.vowels
        sBot(i).sVowel(j).iF = sBot(i).sVowel(j).iF * sParm.agerate;
    end
end

end
