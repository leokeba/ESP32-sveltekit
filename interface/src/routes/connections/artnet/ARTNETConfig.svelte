<script lang="ts">
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import Spinner from '$lib/components/Spinner.svelte';
	import ArtNET from '~icons/tabler/topology-full-hierarchy';
	import Info from '~icons/tabler/info-circle';

	type ArtNETSettings = {
		channel: number;
		universe: number;
	};

	let artNetSettings: ArtNETSettings;

	let formField: any;

	async function getArtNETSettings() {
		try {
			const response = await fetch('/rest/stepperArtNetSettings', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			artNetSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	async function postArtNETSettings() {
		try {
			const response = await fetch('/rest/stepperArtNetSettings', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(artNetSettings)
			});
			if (response.status == 200) {
				notifications.success('ArtNet settings updated.', 3000);
				artNetSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	function handleSubmitArtNet() {
		let valid = true;

		console.log(artNetSettings)

		// Submit JSON to REST API
		if (valid) {
			postArtNETSettings();
			//alert('Form Valid');
		}
	}
</script>

<SettingsCard collapsible={true} open={true}>
	<ArtNET slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">ArtNET Stepper Settings</span>
	<div class="w-full overflow-x-auto">
		{#await getArtNETSettings()}
			<Spinner />
		{:then nothing}
			<form
				on:submit|preventDefault={handleSubmitArtNet}
				novalidate
				bind:this={formField}
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="alert alert-info my-2 shadow-lg">
					<Info class="h-6 w-6 flex-shrink-0 stroke-current" />
					<span>
						The Stepper is controllable via ArtNET using 5 DMX channels : 
						<br />
							{artNetSettings.channel}=Enable,
							{artNetSettings.channel+1}=Direction,
							{artNetSettings.channel+2}=Speed,
							{artNetSettings.channel+3}=Move,
							{artNetSettings.channel+4}=Acceleration
					</span>
				</div>
				<div class="grid w-full grid-cols-2 content-center gap-x-4 px-4">
					<div class="sm:col-span-1">
						<label class="label" for="channel">
							<span class="label-text text-md">DMX Start Channel</span>
						</label>
						<input
							type="number"
							min="1"
							max="508"
							class="input input-bordered invalid:border-error w-full invalid:border-2}"
							bind:value={artNetSettings.channel}
							id="channel"
							required
						/>
					</div>
					<div class="sm:col-span-1">
						<label class="label" for="universe">
							<span class="label-text text-md">DMX Universe</span>
						</label>
						<input
							type="number"
							min="0"
							max="32767"
							class="input input-bordered invalid:border-error w-full invalid:border-2}"
							bind:value={artNetSettings.universe}
							id="universe"
							required
						/>
					</div>
				</div>
				<div class="divider mb-2 mt-0" />
				<div class="mx-4 flex flex-wrap justify-end gap-2">
					<button class="btn btn-primary" type="submit">Apply Settings</button>
				</div>
			</form>
		{/await}
	</div>
</SettingsCard>
